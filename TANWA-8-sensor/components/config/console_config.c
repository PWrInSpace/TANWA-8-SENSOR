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
#include "BoardData.h"
#include "board_config.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_driver.h"

#define TAG "CONSOLE_CONFIG"

extern board_config_t config;

// example function to reset the device
int reset_device(int argc, char **argv) {
    ESP_LOGI(TAG, "Resetting device...");
    esp_restart();
    return 0;
}

static int read_temperature(int argc, char **argv) {

    uint8_t ret = 0;
    float temp;
    temp = BoardData.status_temp;
    CONSOLE_WRITE("TMP1075 Temperature:");
    CONSOLE_WRITE("#1 => temp1 = %f", temp);

    return 0;
}

static int calibrate_sensor(int argc, char **argv) {
    if (argc != 5) {
        CONSOLE_WRITE("Uzycie: cal0 <1-8> <p_max> <v_min> <v_max>");
        return -1;
    }
    
    int user_num = atoi(argv[1]); 
    float p_max = atof(argv[2]);
    float v_min = atof(argv[3]);
    float v_max = atof(argv[4]);

    if (user_num < 1 || user_num > 8) {
        CONSOLE_WRITE("ERROR: Numer czujnika musi byc w zakresie 1-8!");
        return -1;
    }

    int d_idx;
    int s_idx;

    if (user_num <= 4) {
        d_idx = 0;
        s_idx = user_num - 1;
    } else {
        d_idx = 1;
        s_idx = user_num - 5;
    }

    config.pressure_driver[d_idx].sensors[s_idx].pressure_max = p_max;
    config.pressure_driver[d_idx].sensors[s_idx].voltage_min = v_min;
    config.pressure_driver[d_idx].sensors[s_idx].voltage_max = v_max;

    CONSOLE_WRITE("OK: Zaktualizowano RAM dla czujnika %d", user_num);
    CONSOLE_WRITE("Lokalizacja: Driver[%d].Sensor[%d]", d_idx, s_idx);
    CONSOLE_WRITE("Aktualne nastawy: Pmax=%.1f, Vmin=%.3f, Vmax=%.3f", p_max, v_min, v_max);
    CONSOLE_WRITE("Wpisz 'save_cal', aby zapisac na stale we Flash.");

    return 0;
}

static int save_calibration(int argc, char **argv) {
    esp_err_t err =  save_nvs_cal();
    if (err != ESP_OK) {
       ESP_LOGE("NVS","ERROR: Nie udalo sie zapisac kalibracji do NVS!");
        return -1;
    }
     ESP_LOGI("NVS","Kalibracja zapisana do NVS!");
    return 0;
}

 // Place for the console configuration

 static esp_console_cmd_t cmd [] = {
 // example command:
 // cmd     help description   hint  function      args
 {"reset", "Reset the device", NULL, reset_device, NULL, NULL, NULL},
 {"temp-read", "read temperature", NULL, read_temperature, NULL, NULL, NULL},
 {"cal0", "calibrate sensor", NULL, calibrate_sensor, NULL, NULL, NULL},
 {"save_cal", "save calibration data to NVS", NULL, save_calibration, NULL, NULL, NULL},
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