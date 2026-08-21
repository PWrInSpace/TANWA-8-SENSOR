#include "measure_task.h"
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "BoardData.h"
#include "hdc1080.h"
#include "max31856.h"
#include "pressure_driver.h"
#include "sd_task.h"
#include "tmp1075.h"

#define TAG "MEASURE_TASK"
float temperature;
float pressure;
static TaskHandle_t measure_task_handle = NULL;
extern SemaphoreHandle_t mutex_spi_tc;

#define TEMP_LOOP_COUNT 10

esp_err_t measure_task_init(void) {

  if (xTaskCreatePinnedToCore(measure_task, "measure_task", 4096, NULL, 7,
                              &measure_task_handle, 0) == pdPASS) {
    ESP_LOGI("MEASURE_TASK", "Measure task created successfully");
  } else {
    ESP_LOGE("MEASURE_TASK", "Failed to create measure task");
    return ESP_FAIL;
  }

  return ESP_OK;
}

void measure_task(void *) {
  size_t loop_counter = 0;

  while (1) {
    if (xSemaphoreTake(BoardDataSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) {


      pressure_driver_read_pressures(&(config.pressure_driver[0]),
                                     BoardData.pressure, BoardData.voltage);
      pressure_driver_read_pressures(&(config.pressure_driver[1]),
                                     BoardData.pressure + 4,
                                     BoardData.voltage + 4);

      if (loop_counter > TEMP_LOOP_COUNT) {
        tmp1075_status_t ret =
            tmp1075_get_temp_celsius(&(config.tmp1075), &BoardData.status_temp);
        xSemaphoreTake(mutex_spi_tc, portMAX_DELAY);
        BoardData.temperature[0] =
            (thermocouple_read_temperature(&config.thermocouple[0]));
        BoardData.temperature[1] =
            (thermocouple_read_temperature(&config.thermocouple[1]));
        BoardData.temperature[2] =
            (thermocouple_read_temperature(&config.thermocouple[2]));
        xSemaphoreGive(mutex_spi_tc);

        loop_counter = 0;
      } else
        loop_counter++;

      BoardData.timestamp = esp_timer_get_time();
      SDT_send_data(&BoardData, sizeof(BoardData));

      // print_pressures();

      // PRESSURES DATA
      //  N2 ZF | DRD N2 | DRD N20 | BLANK | CUT-OFF | N2 ZR | N2 PR | N2O ZF

      // printf("################################Thermocouple###################################\n");
      // printf("Termocouple_1  FILL N20= %f\n", BoardData.temperature[0]);
      //  printf("Termocouple_2 = SCIANKA %f\n", BoardData.temperature[1]);
      //  printf("Termocouple_3 = %f\n", BoardData.temperature[2]);
      // hdc1080_read_temperature(&config.hdc);
      xSemaphoreGive(BoardDataSemaphore);
      vTaskDelay(pdMS_TO_TICKS(1));

    } else {
      ESP_LOGE(TAG, "Failed to take BoardDataSemaphore");
    }
  }
}