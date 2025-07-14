#include <stdio.h>
#include "measure_task.h"

#include "esp_log.h"
#include "esp_err.h"


#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "tmp1075.h"

float temperature;
static TaskHandle_t measure_task_handle = NULL;
esp_err_t measure_task_init(void) {
    
    if(xTaskCreatePinnedToCore(measure_task, "measure_task", 4096, NULL, 0, &measure_task_handle, 0) == pdPASS) {
        ESP_LOGI("MEASURE_TASK", "Measure task created successfully");
    } else {
        ESP_LOGE("MEASURE_TASK", "Failed to create measure task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void measure_task(void*){

    while(1){
        tmp1075_status_t ret = tmp1075_get_temp_celsius(&(config.tmp1075), &temperature);
        printf("TEMP_STAT = %f", temperature);
        vTaskDelay(500);
    }
}

