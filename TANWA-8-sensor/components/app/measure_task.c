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
#include "hdc1080.h"

#define TAG "MEASURE_TASK"
float temperature;
float pressure;
static TaskHandle_t measure_task_handle = NULL;
esp_err_t measure_task_init(void) {
    
    if(xTaskCreatePinnedToCore(measure_task, "measure_task", 4096, NULL, 7, &measure_task_handle, 0) == pdPASS) {
        ESP_LOGI("MEASURE_TASK", "Measure task created successfully");
    } else {
        ESP_LOGE("MEASURE_TASK", "Failed to create measure task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void print_pressures() {
    printf("\n\033[1;36m/==================== PRESSURE MONITOR ====================\\\033[0m\n");
    printf("\033[1;36m| ID |       SENSOR NAME       |      VALUE [bar]      |\033[0m\n");
    printf("\033[1;36m|----|-------------------------|-----------------------|\033[0m\n");

    
    printf("| P1 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "CUT-OFF N2O",      BoardData.pressure[4]);
    printf("| P2 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "N2O ZA FILLEM",    BoardData.pressure[7]);
    printf("| P3 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "N2 PR",           BoardData.pressure[6]);
    printf("| P4 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "N2 ZR",           BoardData.pressure[5]);
    printf("| P5 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "N2 ZF",           BoardData.pressure[0]);
    printf("| P6 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "BLANK",           BoardData.pressure[3]);
    printf("| P7 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "DRD N2O",         BoardData.pressure[2]);
    printf("| P8 | %-23s | \033[1;33m%10.3f bar\033[0m |\n", "DRD N2",          BoardData.pressure[1]);

    printf("\033[1;36m\\==========================================================/\033[0m\n\n");
}

void measure_task(void*){

         while(1){
         if (xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
    
        tmp1075_status_t ret = tmp1075_get_temp_celsius(&(config.tmp1075), &BoardData.status_temp);
        pressure_driver_read_pressures(&(config.pressure_driver[1]),BoardData.pressure);
        float pressure = 0;
        float pressure2 = 0;
        float pressure3 = 0;
        float pressure4 = 0;
        //pressure_driver_read_pressure(&(config.pressure_driver[0]), 0,&pressure);
        //pressure_driver_read_pressure(&(config.pressure_driver[0]), 1,&pressure2);
        //pressure_driver_read_pressure(&(config.pressure_driver[0]), 2,&pressure3);
        //pressure_driver_read_pressure(&(config.pressure_driver[0]), 3,&pressure4);
        pressure_driver_read_pressures(&(config.pressure_driver[0]),BoardData.pressure);
        pressure_driver_read_pressures(&(config.pressure_driver[1]),BoardData.pressure + 4);
        BoardData.temperature[0] = (thermocouple_read_temperature(&config.thermocouple[0]));
        BoardData.temperature[1]= (thermocouple_read_temperature(&config.thermocouple[1]));
        BoardData.temperature[2] = (thermocouple_read_temperature(&config.thermocouple[2]));

        //printf("################################TEMP_STAT###################################\n");
        //printf("TEMP_STAT = %f\n", BoardData.status_temp);
        //PRESSURES DATA
        // N2 ZF | DRD N2 | DRD N20 | BLANK | CUT-OFF | N2 ZR | N2 PR | N2O ZF 

        

        //printf("################################Thermocouple###################################\n");
       // printf("Termocouple_1  FILL N20= %f\n", BoardData.temperature[0]);
      //  printf("Termocouple_2 = SCIANKA %f\n", BoardData.temperature[1]);
      //  printf("Termocouple_3 = %f\n", BoardData.temperature[2]);
       // hdc1080_read_temperature(&config.hdc);
        xSemaphoreGive(BoardDataSemaphore);
        print_pressures();
        vTaskDelay(pdMS_TO_TICKS(500));

        
    } else 
    {
        ESP_LOGE(TAG, "Failed to take BoardDataSemaphore");
    }
}
}