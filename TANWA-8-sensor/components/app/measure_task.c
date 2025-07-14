#include <stdio.h>
#include "measure_task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_mac.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "tmp1075.h"
#include "pressure_driver.h"
#include "max31856.h"
#include "BoardData.h"
#define TAG "MEASURE_TASK"
float temperature;
float pressure;
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
         if (xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
      
            printf("Temperature sensor\n");
        tmp1075_status_t ret = tmp1075_get_temp_celsius(&(config.tmp1075), &BoardData.status_temp);
        printf("TEMP_STAT = %f\n", BoardData.status_temp);

        printf("###################################################################\n");
        printf("Pressure sensor\n");
        pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_1,&BoardData.pressure[0]);
        printf("PRESS_STAT_1 = %f\n", BoardData.pressure[0]);

        pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_2,&BoardData.pressure[1]);
        printf("PRESS_STAT_2 = %f\n", BoardData.pressure[1]);

        pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_3,&BoardData.pressure[2]);
        printf("PRESS_STAT_3 = %f\n", BoardData.pressure[2]);

        pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_4,&BoardData.pressure[3]);
        printf("PRESS_STAT_4 = %f\n", BoardData.pressure[3]);
        
        printf("###################################################################\n");
        printf("Thermocouple\n");

        BoardData.temperature[0] = (thermocouple_read_temperature(&config.thermocouple[0]));
        BoardData.temperature[1]= (thermocouple_read_temperature(&config.thermocouple[1]));
        BoardData.temperature[2] = (thermocouple_read_temperature(&config.thermocouple[2]));
        
        printf("Termocouple 1 = %f\n", BoardData.temperature[0]);
       printf("Termocouple 2 = %f\n", BoardData.temperature[1]);
        printf("Termocouple 3 = %f\n", BoardData.temperature[2]);
        vTaskDelay(50);
        xSemaphoreGive(BoardDataSemaphore);
    } else 
    {
        ESP_LOGE(TAG, "Failed to take BoardDataSemaphore");
    }
}
}