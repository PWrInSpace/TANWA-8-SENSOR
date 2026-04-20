#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "setup_task.h"
#include "board_config.h"
#include "tmp1075.h"
#include "max31856.h"

#include "measure_task.h"
#include "sd_card_config.h"

#define TAG "APP"
extern board_config_t config;
esp_err_t setup_task_init(void);

static TaskHandle_t setup_task_handle = NULL;

void setup_task(void *arg) {
    esp_err_t err;

    err = board_config_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Board configuration failed");
        vTaskDelete(NULL);
    }   

    measure_task_init();
    ESP_LOGI(TAG, "SETUP DONE");

    vTaskDelete(NULL);
}

esp_err_t setup_task_init(void) {
    if (xTaskCreatePinnedToCore(setup_task, "setup_task", 4096, NULL, 0, &setup_task_handle, 0) == pdPASS) {
        ESP_LOGI(TAG, "Setup task created successfully");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Failed to create setup task");
        return ESP_FAIL;
    }
}

esp_err_t setup_task_deinit(void) {
    if (setup_task_handle != NULL) {
        vTaskDelete(setup_task_handle);
        setup_task_handle = NULL;
    }
    
    return ESP_OK;
}