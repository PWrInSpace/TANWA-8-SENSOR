#include "data_board_task.h"
#include "pressure_driver.h"

#define TAG "DATA_BOARD_TASK"

void data_board_task(void *arg) {
    // Take semaphore with timeout
   
    }

    vTaskDelete(NULL); // Delete task when done, if intended to run once
}