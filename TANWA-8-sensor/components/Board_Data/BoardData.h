#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "stdbool.h"


typedef struct {
    float temperature[3];
    float status_temp;
    float pressure[8];

} BoardData_t;

extern BoardData_t BoardData;
extern SemaphoreHandle_t BoardDataSemaphore;

esp_err_t board_data_init(void);