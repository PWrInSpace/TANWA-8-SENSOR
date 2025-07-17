///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//

#include "esp_log.h"
#include "esp_system.h"

#include "console_config.h"
#include "console.h"

#define TAG "CONSOLE_CONFIG"

#include "esp_console.h"
#include "console.h"
#include "console_config.h"

#define TAG "CONSOLE_CONFIG"


// example function to reset the device
int reset_device(int argc, char **argv) {
    ESP_LOGI(TAG, "Resetting device...");
    esp_restart();
    return 0;
}

static int read_temperature(int argc, char **argv) {

    uint8_t ret = 0;
    float temp;

   // ret = tmp1075_get_temp_celsius(&(config.tmp1075), &temp);
    //if (ret != TMP1075_OK) {
    //    ESP_LOGE(TAG, "TMP1075 #1 read temp failed - status: %d", ret);
     //   return -1;
   // }
    
    CONSOLE_WRITE("TMP1075 Temperature:");
   //CONSOLE_WRITE("#1 => temp1 = %f", temp);

    return 0;
}



 // Place for the console configuration

 static esp_console_cmd_t cmd [] = {
 // example command:
 // cmd     help description   hint  function      args
 {"reset", "Reset the device", NULL, reset_device, NULL, NULL, NULL},
 {"temp-read", "read temperature", NULL, read_temperature, NULL, NULL, NULL},
 };

 esp_console_config_t console_config = {
    .max_cmdline_args = 8,
    .max_cmdline_length = 256,
    .hint_color = 36
};


esp_err_t console_config_init() {
    esp_err_t ret;
    ret = console_init();
    ret = console_register_commands(cmd, sizeof(cmd) / sizeof(cmd[0]));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(ret));
        return ret;
    }
    return ret;
}