#include "sd_task.h"

#include "esp_log.h"
#define TAG "SDT"

static struct {
    sd_card_t sd_card;

    TaskHandle_t sd_task;
    RingbufHandle_t data_ringbuffer;
    RingbufHandle_t log_ringbuffer;

    SemaphoreHandle_t ringbuffer_mutex;
    SemaphoreHandle_t data_write_mutex;  // prevent race condition during path changing
    SemaphoreHandle_t spi_mutex;

    size_t data_items_count;

    void *data_from_queue;
    size_t data_from_queue_size;
    char data_buffer[SD_DATA_BUFFER_MAX_SIZE];
    char log_buffer[SD_LOG_BUFFER_MAX_SIZE];

    char data_path[SD_PATH_SIZE];
    char log_path[SD_PATH_SIZE];

    uint32_t try_to_mount_counter;

    error_handler error_handler_fnc;
    create_sd_frame create_sd_frame_fnc;
} mem = {
    .sd_task = NULL,
    .log_ringbuffer = NULL,
    .data_ringbuffer = NULL,
    .data_write_mutex = NULL,
    .ringbuffer_mutex = NULL
};

static void report_error(SD_TASK_ERR error_code) {
    if (mem.error_handler_fnc == NULL) {
        return;
    }

    mem.error_handler_fnc(error_code);
}

static bool write_to_sd(FILE *file, char *data, size_t size) {
    if (file == NULL) {
        return false;
    }

    xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
    fwrite(data, 1, size, file);
    xSemaphoreGive(mem.spi_mutex);

    return true;
}

static void prepare_data_file_and_save(void) {
    xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
    FILE *data_file = fopen(mem.data_path, "a");
    xSemaphoreGive(mem.spi_mutex);

    if (data_file == NULL) {
        ESP_LOGE(TAG, "FILE OPEN ERROR %s", mem.data_path);
        xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
        SD_remount(&mem.sd_card);
        xSemaphoreGive(mem.spi_mutex);
        report_error(SD_WRITE);
        return;
    }

    int received_data_counter = 0;
    while (received_data_counter < SD_MAX_DATA_RECEIVE) {
        size_t item_size;
        void *item = xRingbufferReceive(mem.data_ringbuffer, &item_size, 0);
        if (item == NULL) break;
        
        size_t frame_size = mem.create_sd_frame_fnc(mem.data_buffer, sizeof(mem.data_buffer), item, item_size);
        if (write_to_sd(data_file, mem.data_buffer, frame_size) == false) {
            xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
            SD_remount(&mem.sd_card);
            xSemaphoreGive(mem.spi_mutex);
            report_error(SD_WRITE);
        }

        vRingbufferReturnItem(mem.data_ringbuffer, item);
        xSemaphoreTake(mem.ringbuffer_mutex, portMAX_DELAY);
        mem.data_items_count--;
        xSemaphoreGive(mem.ringbuffer_mutex);
        
        received_data_counter++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
    fclose(data_file);
    xSemaphoreGive(mem.spi_mutex);
}

static bool check_sd_status(void) {
    if (mem.sd_card.mounted == true) {
        return true;
    }

    if (mem.try_to_mount_counter < SD_TRY_TO_REMOUNT_DELAY) {
        mem.try_to_mount_counter++;
        return false;
    }
    mem.try_to_mount_counter = 0;

    xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
    bool result = SD_mount(&mem.sd_card);
    xSemaphoreGive(mem.spi_mutex);
    
    void *item;
    size_t item_size;
    while ((item = xRingbufferReceive(mem.data_ringbuffer, &item_size, 0)) != NULL) {
        vRingbufferReturnItem(mem.data_ringbuffer, item);
    }
    xSemaphoreTake(mem.ringbuffer_mutex, portMAX_DELAY);
    mem.data_items_count = 0;
    xSemaphoreGive(mem.ringbuffer_mutex);

    return result;
}

static bool data_has_at_least(size_t n) {
    xSemaphoreTake(mem.ringbuffer_mutex, portMAX_DELAY);
    bool ok = (mem.data_items_count >= n);
    xSemaphoreGive(mem.ringbuffer_mutex);
    return ok;
}

static void data_check_and_save(void) {
    if (check_sd_status() == false) {
        return;
    }

    if (!data_has_at_least(SD_DATA_DROP_VALUE)) {
        return;
    }

    prepare_data_file_and_save();
}

static void log_check_and_save(void) {
    if (check_sd_status() == false) {
        return;
    }

    int received_data_counter = 0;
    while (received_data_counter < SD_MAX_DATA_RECEIVE) {
        size_t item_size;
        const char *item = (const char *)xRingbufferReceive(mem.log_ringbuffer, &item_size, 0);
        if (item == NULL) break;

        xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
        if (SD_write(&mem.sd_card, mem.log_path, item, item_size) == false) {
            report_error(SD_WRITE);
        }
        xSemaphoreGive(mem.spi_mutex);

        vRingbufferReturnItem(mem.log_ringbuffer, (void *)item);

        received_data_counter++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void terminate_task(void) {
    ESP_LOGI(TAG, "Terminating SD TASK");
    vRingbufferDelete(mem.data_ringbuffer);
    vRingbufferDelete(mem.log_ringbuffer);
    mem.data_ringbuffer = NULL;
    mem.log_ringbuffer = NULL;
    free(mem.data_from_queue);
    vTaskDelete(NULL);
}

static void check_terminate_condition(void) {
    if (ulTaskNotifyTake(pdTRUE, 0) == 0) {
        return;
    }

    prepare_data_file_and_save();
    log_check_and_save();
    terminate_task();
}

static void sdTask(void *args) {
    ESP_LOGI(TAG, "RUNNING SD TASK");
    {
        UBaseType_t high = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "SD task stack high water mark: %u", (unsigned)high);
    }
    while (1) {
        if (xSemaphoreTake(mem.data_write_mutex, 10) == pdTRUE) {
            data_check_and_save();
            xSemaphoreGive(mem.data_write_mutex);
        } else {
            report_error(SD_MUTEX);
        }

        log_check_and_save();
        check_terminate_condition();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static bool check_if_file_exists(char *path) {
    bool res;
    xSemaphoreTake(mem.spi_mutex, portMAX_DELAY);
    res = SD_file_exists(path);
    xSemaphoreGive(mem.spi_mutex);
    return res;
}

static bool create_unique_path(char *path, size_t size) {
    char temp_path[SD_PATH_SIZE] = {0};
    int ret = 0;
    for (int i = 0; i < 1000; ++i) {
        ret = snprintf(temp_path, sizeof(temp_path), SD_MOUNT_POINT "/%s%d.txt", path, i);
        if (ret == SD_PATH_SIZE) {
            return false;
        }

        if (check_if_file_exists(temp_path) == false) {
            memcpy(path, temp_path, size);
            return true;
        }
    }
    return false;
}

static bool initialize_sd_card(sd_task_cfg_t *task_cfg) {
    sd_card_config_t card_cfg = {
        .spi_host = task_cfg->spi_host,
        .cs_pin = task_cfg->cs_pin,
        .mount_point = SDCARD_MOUNT_POINT,
    };

    bool ret = SD_init(&mem.sd_card, &card_cfg);
    if (ret == false) {
        ESP_LOGW(TAG, "Unable to initialize SD card");
        report_error(SD_INIT);
        return false;
    }

    mem.error_handler_fnc = task_cfg->error_handler_fnc;

    memcpy(mem.data_path, task_cfg->data_path, task_cfg->data_path_size);
    memcpy(mem.log_path, task_cfg->log_path, task_cfg->log_path_size);
    if (create_unique_path(mem.data_path, sizeof(mem.data_path)) == false) {
        ESP_LOGE(TAG, "Unable to create unique path");
    }

    if (create_unique_path(mem.log_path, sizeof(mem.log_path)) == false) {
        ESP_LOGE(TAG, "Unable to create unique path");
    }

    ESP_LOGI(TAG, "Using data path %s", mem.data_path);
    ESP_LOGI(TAG, "Using log path %s", mem.log_path);

    return true;
}

static bool initialize_task(sd_task_cfg_t *task_cfg) {
    if (task_cfg == NULL || task_cfg->create_sd_frame_fnc == NULL) {
        return false;
    }
    mem.create_sd_frame_fnc = task_cfg->create_sd_frame_fnc;

    mem.data_from_queue_size = task_cfg->data_size;
    mem.data_from_queue = malloc(task_cfg->data_size);
    if (mem.data_from_queue == NULL) {
        return false;
    }
    
    // nie ma gwarancji, że w buforze będzie dokładnie CONFIG_SD_DATA_RINGBUF_CAPACITY itemów, bo RINGBUF_TYPE_NOSPLIT dokłada do każdego itemu jeszcze header (8 bajtów) oraz każdy item jest trzymany w blokach po 32 bity (niezaleznie od rzeczywistego rozmiaru)
    mem.data_ringbuffer = xRingbufferCreate((CONFIG_SD_DATA_RINGBUF_CAPACITY * (task_cfg->data_size)), RINGBUF_TYPE_NOSPLIT);
    if (mem.data_ringbuffer == NULL) {
        free(mem.data_from_queue);
        return false;
    }
    mem.data_items_count = 0;

    mem.log_ringbuffer = xRingbufferCreate(SD_LOG_RINGBUF_SIZE, RINGBUF_TYPE_NOSPLIT);
    if (mem.log_ringbuffer == NULL) {
        vRingbufferDelete(mem.data_ringbuffer);
        mem.data_ringbuffer = NULL;
        free(mem.data_from_queue);
        return false;
    }

    mem.ringbuffer_mutex = xSemaphoreCreateMutex();
    if (mem.ringbuffer_mutex == NULL) {
        vRingbufferDelete(mem.data_ringbuffer);
        vRingbufferDelete(mem.log_ringbuffer);
        mem.data_ringbuffer = NULL;
        mem.log_ringbuffer = NULL;
        free(mem.data_from_queue);
        return false;
    }

    // prevent race condition during path changing
    mem.data_write_mutex = xSemaphoreCreateMutex();
    if (mem.data_write_mutex == NULL) {
        vRingbufferDelete(mem.data_ringbuffer);
        vRingbufferDelete(mem.log_ringbuffer);
        mem.data_ringbuffer = NULL;
        mem.log_ringbuffer = NULL;
        vSemaphoreDelete(mem.ringbuffer_mutex);
        mem.ringbuffer_mutex = NULL;
        free(mem.data_from_queue);
        return false;
    }

    xTaskCreatePinnedToCore(sdTask, "sd task", task_cfg->stack_depth, NULL, task_cfg->priority,
                            &mem.sd_task, task_cfg->core_id);

    if (mem.sd_task == NULL) {
        vRingbufferDelete(mem.data_ringbuffer);
        vRingbufferDelete(mem.log_ringbuffer);
        mem.data_ringbuffer = NULL;
        mem.log_ringbuffer = NULL;
        vSemaphoreDelete(mem.ringbuffer_mutex);
        vSemaphoreDelete(mem.data_write_mutex);
        mem.ringbuffer_mutex = NULL;
        mem.data_write_mutex = NULL;
        free(mem.data_from_queue);
        return false;
    }

    return true;
}

bool SDT_init(sd_task_cfg_t *task_cfg) {
    mem.spi_mutex = task_cfg->spi_mutex;

    if (initialize_sd_card(task_cfg) == false) {
        ESP_LOGE(TAG, "Unable to initialzie sd card");
        return false;
    }

    if (initialize_task(task_cfg) == false) {
        ESP_LOGE(TAG, "Unable to initialzie sd task");
        return false;
    }

    return true;
}

bool SDT_send_data(void *data, size_t data_size) {
    if (mem.data_ringbuffer == NULL) {
        return false;
    }

    if (data_size != mem.data_from_queue_size) {
        return false;
    }

    if (mem.sd_card.mounted == false) {
        return false;
    }

    if (xRingbufferSend(mem.data_ringbuffer, data, data_size, 0) == pdFALSE) {
        ESP_LOGW(TAG, "Unable to add data to sd mem.data_ringbuffer");
        return false;
    }
    xSemaphoreTake(mem.ringbuffer_mutex, portMAX_DELAY);
    mem.data_items_count++;
    xSemaphoreGive(mem.ringbuffer_mutex);

    return true;
}

bool SDT_send_log(char *data, size_t data_size) {
    if (mem.log_ringbuffer == NULL) {
        return false;
    }

    if (data_size > SD_DATA_BUFFER_MAX_SIZE) {
        return false;
    }

    if (xRingbufferSend(mem.log_ringbuffer, data, data_size, 0) == pdFALSE) {
        ESP_LOGW(TAG, "Unable to add data to sd mem.log_ringbuffer");
        return false;
    }

    return true;
}

bool SDT_change_data_path(char *new_path, size_t path_size) {
    if (xSemaphoreTake(mem.data_write_mutex, 100) == pdFALSE) {
        return false;
    }

    memcpy(mem.data_path, new_path, path_size);
    if (create_unique_path(mem.data_path, sizeof(mem.data_path)) == false) {
        ESP_LOGE(TAG, "Unable to create unique path");
    }

    xSemaphoreGive(mem.data_write_mutex);
    return true;
}

void SDT_terminate_task(void) { xTaskNotifyGive(mem.sd_task); }
