#include <stdio.h>

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