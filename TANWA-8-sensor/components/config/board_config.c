///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Szymon Rzewuski
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//
#include <stdbool.h>
#include <stdint.h>
#include "board_config.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"


#include "mcu_gpio_config.h"
#include "mcu_twai_config.h"
#include "can_config.h"
#include "BoardData.h"
#include "console_config.h"
#include "flash.h"

#define TAG "BOARD_CONFIG"


//Hardware include

#include "mcu_twai_config.h"
#include "esp_log.h"
#include "pressure_driver.h"
#include "mcu_gpio_config.h"
#include "mcu_i2c_config.h"
#include "mcu_spi_config.h"
#include "sd_card_config.h"
#include "pinout.h"

#define IOEXP_MODE (IOCON_INTCC | IOCON_INTPOL | IOCON_ODR | IOCON_MIRROR)

#define CONFIG_I2C_TMP1075_TS1_ADDR 0x4C // TODO: ADD ADRESS1
#define CONFIG_I2C_TMP1075_TS2_ADDR 0x4E // TODO: ADD ADRESS2

#include "ads1115.h"
#include "pressure_driver.h"
#include "max31856.h"
#include "hdc1080.h"

void _led_delay(uint32_t _ms) {
    vTaskDelay(_ms / portTICK_PERIOD_MS);
}

board_config_t config = 
{
    .board_name = "TANWA_BOARD", //CHANGE TO REAL BOARD NAME
    .status_led = {
        ._gpio_set_level = _mcu_gpio_set_level,
        ._delay = _led_delay,

        .gpio_num = LED_GPIO_INDEX,
        .drive = LED_DRIVE_POSITIVE,
        .state = LED_STATE_OFF, 
    },
    .tmp1075 = {
    
        ._i2c_write = _mcu_i2c_write,
        ._i2c_read = _mcu_i2c_read,
        .i2c_address = CONFIG_I2C_TMP1075_TS1_ADDR,
        .config_register = 0,
    },
    .ads1115[0] = {
        ._i2c_write = _mcu_i2c_write,
        ._i2c_read = _mcu_i2c_read,
        .i2c_address = 0x49,
    },
    .ads1115[1] = {
        ._i2c_write = _mcu_i2c_write,
        ._i2c_read = _mcu_i2c_read,
        .i2c_address = 0x48,
    },
    .pressure_driver[0] = PRESSURE_DRIVER_TANWA_CONFIG1(&config.ads1115[0]),
    .pressure_driver[1] = PRESSURE_DRIVER_TANWA_CONFIG2(&config.ads1115[1]),
    .hdc = {
        .i2c_address = HDC1080_I2C_ADDRESS,
        ._i2c_write = _mcu_i2c_write,
        ._i2c_read = _mcu_i2c_read,
        .temperature = 50,
        .humidity_percentage = 99,
    },
};

esp_err_t press_sensors_init(void); // code below board_config_init()

esp_err_t board_config_init(void) {

    esp_err_t err;
    err = mcu_gpio_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO initialization failed");
        return err;
    }

    board_data_init(); //!MUST BE BEFORE CAN TASK BECAUSE CAN TASK IS USING IT

    err = mcu_twai_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "TWAI initialization failed");
        return err;
    }

    err = can_config_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN initialization failed");
        return err;
    }

    err = flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Flash/NVS initialization failed");
        return err;
    }
    
    err = mcu_i2c_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C failed");
        return err;
    }
    ESP_LOGI(TAG, "I2C init successful");
    
    err = mcu_spi_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI failed");
        return err;
    }
    ESP_LOGI(TAG, "SPI init successful");
    
    err = press_sensors_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Pressure sensors initialization failed");
        return err;
    }
    ESP_LOGI(TAG, "Pressure sensors init successful");
    
    err = console_config_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Console initialization failed");
        return err;
    }

    bool ret = initialize_sd();
    if (ret != true) {
        ESP_LOGE(TAG, "SD card initialization failed");
        return err;
    }
    ESP_LOGI(TAG, "SD card init successful");
    
    // INIT THERMOCOUPLES
    uint8_t fault_val;
    
    ESP_LOGI(TAG, "Thermocouple initialization...");
    max31856_init(&config.thermocouple[0], THERMOCOUPLE_CS1);
    ESP_LOGI(TAG, "Thermocouple 1 DONE INIT");
    max31856_init(&config.thermocouple[1], THERMOCOUPLE_CS2);
    ESP_LOGI(TAG, "Thermocouple 2 DONE INIT");
    max31856_init(&config.thermocouple[2], THERMOCOUPLE_CS3);
    ESP_LOGI(TAG, "Thermocouple set type...");

    thermocouple_set_type(&config.thermocouple[0], MAX31856_TCTYPE_K);
    thermocouple_set_type(&config.thermocouple[1], MAX31856_TCTYPE_K);
    thermocouple_set_type(&config.thermocouple[2], MAX31856_TCTYPE_K);
    ESP_LOGI(TAG, "Thermocouple read fault...");

    fault_val = thermocouple_read_fault(&config.thermocouple[0], true);
    if (fault_val == 1) {
        return ESP_FAIL;
    }

    fault_val = thermocouple_read_fault(&config.thermocouple[1], true);
    if (fault_val == 1) {
       return ESP_FAIL;
    }

    fault_val = thermocouple_read_fault(&config.thermocouple[2], true);
    if (fault_val == 1) {
        return ESP_FAIL;
    }
    
    //hd1080_init(&config.hdc);
    return ESP_OK;

    //*********** ADD HARDWARE CONFIGURATION HERE ***********//
}

esp_err_t press_sensors_init(void) {
    data_config_t nvs_config;
    esp_err_t err = flash_read(&nvs_config);
    if (err != ESP_OK) return err;
    
    const float *p = (const float *)&nvs_config.press_calibr;
    #define VARIABLES_COUNT 3
    
    #define EXPECTED_CALIBR_SIZE (ADS1115_QUANTITY * PRESSURE_DRIVER_SENSOR_COUNT * VARIABLES_COUNT * sizeof(float))
    static_assert(sizeof(nvs_config.press_calibr) == EXPECTED_CALIBR_SIZE, "Struct padding detected! Pointer arithmetic in calibration loop for pressure sensors will fail.");
    
    for (int i = 0; i < ADS1115_QUANTITY; i++) {
        for (int j = 0; j < PRESSURE_DRIVER_SENSOR_COUNT; j++) {
            int idx = (i * (PRESSURE_DRIVER_SENSOR_COUNT * VARIABLES_COUNT)) + (j * VARIABLES_COUNT);
            
            config.pressure_driver[i].sensors[j].calibr_cfg.voltage_zero = p[idx];
            config.pressure_driver[i].sensors[j].calibr_cfg.voltage_1 = p[idx + 1];
            config.pressure_driver[i].sensors[j].calibr_cfg.pressure_1 = p[idx + 2];
        }
    }
    
    // INIT PRESSURE SENSOR
    pressure_driver_status_t ret_press;
    ret_press = pressure_driver_init(&(config.pressure_driver[0]));
    if (ret_press != PRESSURE_DRIVER_OK) {
        ESP_LOGE(TAG, "Failed to initialize pressure driver");
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Pressure driver 1 initialized");
    }

    ret_press = pressure_driver_init(&(config.pressure_driver[1]));
    if (ret_press != PRESSURE_DRIVER_OK) {
       ESP_LOGE(TAG, "Failed to initialize pressure driver");
       return ESP_FAIL;
    } else {
       ESP_LOGI(TAG, "Pressure driver 2 initialized");
    }

    return ESP_OK;
}