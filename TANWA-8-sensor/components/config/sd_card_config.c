#include "sd_card_config.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "mcu_spi_config.h"
#include "sd_task.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TAG "SD_C"

extern SemaphoreHandle_t mutex_spi;

static size_t convert_data_to_frame(char *buf, size_t buf_size, void* data, size_t size) {

    // rocket_data_t* rocket_data = (rocket_data_t*)data;
    // return pysd_create_sd_frame(buf, buf_size, *rocket_data, true);
    return 0;
}

void on_error(SD_TASK_ERR error) {
    ESP_LOGE(TAG, "!!! SD ERROR CODE: %d !!!", error);
}

bool initialize_sd(void) {
    esp_timer_init();

    sd_task_cfg_t cfg = {
        .cs_pin = SD_CS_Pin,
        .data_path = "data",
        .data_path_size = 9,
        .spi_host = SDSPI_DEFAULT_HOST,
        .log_path = "log",
        .log_path_size = 5,
        .stack_depth = CONFIG_SD_TASK_STACK_DEPTH,
        .priority = CONFIG_SD_TASK_PRIORITY,
        .core_id = CONFIG_SD_TASK_CORE_ID,
        .error_handler_fnc = on_error,
        .data_size = 64, // WARNING! Change to actual size
        .create_sd_frame_fnc = convert_data_to_frame,
        .spi_mutex = mutex_spi,
    };

    return SDT_init(&cfg);
}