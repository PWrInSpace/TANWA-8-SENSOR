#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "setup_task.h"
#include "measure_task.h"
#include "led_driver.h"
#include <stdio.h>

static const char *TAG = "i2cscanner";
#include "board_config.h"
#include "tmp1075.h"


void app_main(void)
{
    // CONFIGURE THE MESSAGE

    ESP_LOGI(TAG, "%s TANWA board starting", config.board_name);
    
    if(setup_task_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize setup task");
        return;
    }

    
    while(1) {
        led_toggle(&(config.status_led));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
