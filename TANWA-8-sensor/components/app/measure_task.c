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
     //  printf("Temperature sensor\n");
        //tmp1075_status_t ret = tmp1075_get_temp_celsius(&(config.tmp1075), &temperature);
       // printf("TEMP_STAT = %f\n", temperature);

      //  printf("###################################################################\n");
      //  printf("Pressure sensor\n");
      //  pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_1,&pressure);
      //  printf("PRESS_STAT_1 = %f\n", pressure);

      //  pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_2,&pressure);
      //  printf("PRESS_STAT_2 = %f\n", pressure);

      //  pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_3,&pressure);
      //  printf("PRESS_STAT_3 = %f\n", pressure);

      //  pressure_driver_read_pressure(&(config.pressure_driver), PRESSURE_DRIVER_SENSOR_4,&pressure);
      //  printf("PRESS_STAT_4 = %f\n", pressure);

      //  printf("###################################################################\n");
      //  printf("Thermocouple\n");

        float temp1 = (thermocouple_read_temperature(&config.thermocouple[0]));
       float temp2 = (thermocouple_read_temperature(&config.thermocouple[1]));
        float temp3 = (thermocouple_read_temperature(&config.thermocouple[2]));
        
        printf("Termocouple 1 = %f\n", temp1);
       printf("Termocouple 2 = %f\n", temp2);
        printf("Termocouple 3 = %f\n", temp3);
        vTaskDelay(50);
    }
}
