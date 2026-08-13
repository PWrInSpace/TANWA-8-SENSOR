#include <stdio.h>
#include "sd_card_config.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "mcu_spi_config.h"
#include "sd_task.h"
#include "BoardData.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TAG "SD_C"

extern SemaphoreHandle_t mutex_spi;

#define SD_CSV_HEADER \
    "temp0,temp1,temp2,status_temp,humidity," \
    "pressure0,pressure1,pressure2,pressure3,pressure4,pressure5,pressure6,pressure7," \
    "voltage0,voltage1,voltage2,voltage3,voltage4,voltage5,voltage6,voltage7\n"

static size_t convert_data_to_frame(char *buf, size_t buf_size, void* data, size_t size) {
    if (buf == NULL || data == NULL || size != sizeof(BoardData_t)) return 0;

    static bool header_written = false;
    BoardData_t *bd = (BoardData_t *)data;
    size_t offset = 0;

    if (header_written == false) {
        int hn = snprintf(buf, buf_size, "%s", SD_CSV_HEADER);
        if (hn < 0 || (size_t)hn >= buf_size) {
            return 0;
        }
        offset = (size_t)hn;
        header_written = true;
    }

    int n = snprintf(buf + offset, buf_size - offset,
        "%.3f,%.3f,%.3f,%.3f,%.3f,"
        "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
        "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
        bd->temperature[0], bd->temperature[1], bd->temperature[2],
        bd->status_temp, bd->humidity,
        bd->pressure[0], bd->pressure[1], bd->pressure[2], bd->pressure[3],
        bd->pressure[4], bd->pressure[5], bd->pressure[6], bd->pressure[7],
        bd->voltage[0], bd->voltage[1], bd->voltage[2], bd->voltage[3],
        bd->voltage[4], bd->voltage[5], bd->voltage[6], bd->voltage[7]);

    if (n < 0 || (size_t)n >= buf_size - offset) return offset;
    return offset + (size_t)n;
}

void on_error(SD_TASK_ERR error) {
    ESP_LOGE(TAG, "!!! SD ERROR CODE: %d !!!", error);
}

bool initialize_sd(void) {
    esp_timer_init();

    sd_task_cfg_t cfg = {
        .cs_pin = SD_CS_Pin,
        .data_path = "data",
        .data_path_size = 9,
        .spi_host = SDSPI_DEFAULT_HOST,
        .log_path = "log",
        .log_path_size = 5,
        .stack_depth = CONFIG_SD_TASK_STACK_DEPTH,
        .priority = CONFIG_SD_TASK_PRIORITY,
        .core_id = CONFIG_SD_TASK_CORE_ID,
        .error_handler_fnc = on_error,
        .data_size = sizeof(BoardData_t),
        .create_sd_frame_fnc = convert_data_to_frame,
        .spi_mutex = mutex_spi,
    };

    return SDT_init(&cfg);
}