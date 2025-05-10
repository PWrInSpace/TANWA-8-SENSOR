#include <stdio.h>
#include "measure_task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "hardware_config.h"

esp_err_t measure_data(termo_data_t *termo_data){

    if(hardware_config.is_initialized == 0){
        ESP_LOGE(TAG, "Hardware is not initialized");
        return ESP_FAIL;
    }

    termo_data->temperature = termocouple_read_temperature(&hardware_config.termocouple);

    return ESP_OK;

}

void measure_task(void){

    TickType_t xLastWakeTime = xTaskGetTickCount();

    // termo_cmds_struct_t termo_cmd = {TERMO_MEASURE, 0};

    while(1){

        // if(xQueueReceive(termo_cmds_queue, &termo_cmd, 0) != pdTRUE){
        //     ESP_LOGI(TAG, "Command parsed");
        // }

        // parse_command(&termo_cmd);

        // termo_cmd.command = TERMO_MEASURE;

        // if(xQueueSend(termo_cmds_queue, &termo_cmd, 0) != pdTRUE){
        //     ESP_LOGE(TAG, "Failed to send termo_MEASURE to termo_cmds_queue");
        // }

        // set_board_status(BOARD_OK);



        vTaskDelayUntil(&xLastWakeTime, MEASURE_TASK_FREQENCY_MS / portTICK_PERIOD_MS);
    }
}