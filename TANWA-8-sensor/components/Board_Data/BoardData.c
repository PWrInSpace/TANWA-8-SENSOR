#include "BoardData.h"
#include <esp_err.h>

BoardData_t BoardData;
SemaphoreHandle_t BoardDataSemaphore;

esp_err_t board_data_init(void) {
    // Initialize semaphore
    BoardDataSemaphore = xSemaphoreCreateMutex();
    if (BoardDataSemaphore == NULL) {
        return ESP_ERR_NO_MEM;
    }
    // Initialize board data (valves are initialized in valves_init())
    return ESP_OK;
}