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
#include "console_config.h"

#define TAG "BOARD_CONFIG"


//Hardware include

#include "mcu_twai_config.h"
#include "esp_log.h"
#include "pressure_driver.h"
#include "mcu_gpio_config.h"
#include "mcu_i2c_config.h"
#include "mcu_spi_config.h"
#include "pinout.h"

#define IOEXP_MODE (IOCON_INTCC | IOCON_INTPOL | IOCON_ODR | IOCON_MIRROR)

#define CONFIG_I2C_TMP1075_TS1_ADDR 0x4C // TODO: ADD ADRESS1
#define CONFIG_I2C_TMP1075_TS2_ADDR 0x4E // TODO: ADD ADRESS2

#include "ads1115.h"
#include "pressure_driver.h"
#include "max31856.h"

void _led_delay(uint32_t _ms) {
    vTaskDelay(_ms / portTICK_PERIOD_MS);
}

board_config_t config = {
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
    .ads1115 = {
        ._i2c_write = _mcu_i2c_write,
        ._i2c_read = _mcu_i2c_read,
        .i2c_address = 0x49,
    },
    .pressure_driver = PRESSURE_DRIVER_TANWA_CONFIG(&config.ads1115),

    

};

esp_err_t board_config_init(void) {

    esp_err_t err;
    
    err = mcu_gpio_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO initialization failed");
        return err;
    }

    //err = mcu_twai_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "TWAI initialization failed");
        return err;
    }

    //err = can_config_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN initialization failed");
        return err;
    }

    err = console_config_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Console initialization failed");
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
    //*********** ADD HARDWARE CONFIGURATION HERE ***********//
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
    if (fault_val == 1)
    {
        return ESP_FAIL;
    }
    fault_val = thermocouple_read_fault(&config.thermocouple[1], true);
    if (fault_val == 1)
    {
       return ESP_FAIL;
    }
   fault_val = thermocouple_read_fault(&config.thermocouple[2], true);
    if (fault_val == 1)
   {
        return ESP_FAIL;
    }
        
        

    // INIT PRESSURE SENSOR
    pressure_driver_status_t ret_press;
    ret_press = pressure_driver_init(&(config.pressure_driver));
    if (ret_press != PRESSURE_DRIVER_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize pressure driver");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "Pressure driver initialized");
    }

    return ESP_OK;
    
}