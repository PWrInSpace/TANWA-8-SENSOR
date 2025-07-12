///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 22.01.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
//#include "driver/i2c_master.h"
#include "mcu_i2c_config.h"
#include <string.h>

#define TAG "MCU_I2C"

static mcu_i2c_config_t mcu_i2c_config = {
  .port = CONFIG_I2C_MASTER_PORT_NUM, 
  .sda = SDA_GPIO, 
  .scl = SCL_GPIO,
  .clk_speed = CONFIG_I2C_MASTER_FREQUENCY, 
  .i2c_init_flag = false,           
};

esp_err_t mcu_i2c_init() {
  if (mcu_i2c_config.i2c_init_flag == false) {
      i2c_config_t conf;
      conf.mode = I2C_MODE_MASTER;
      conf.sda_io_num = SDA_GPIO;
      conf.scl_io_num = SCL_GPIO;
      conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
      conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
      conf.master.clk_speed = 100000;
      i2c_param_config(I2C_NUM_0, &conf);

      
    esp_err_t ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "I2C driver install error: %d", ret);
      return ret;
    }
    mcu_i2c_config.i2c_init_flag = true;
  }
  return ESP_OK;
}

bool _mcu_i2c_write(uint8_t address, uint8_t reg, uint8_t *data, uint8_t len) {
    esp_err_t ret;
    // dynamically create a buffer to hold the data to be written
    uint8_t *write_buf = malloc(len + 1);
    write_buf[0] = reg;
    memcpy(write_buf + 1, data, len);
    ret = i2c_master_write_to_device(CONFIG_I2C_MASTER_PORT_NUM, address, write_buf, len+1, CONFIG_I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    free(write_buf);
    return (bool)(ret == ESP_OK);
}

bool _mcu_i2c_read(uint8_t address, uint8_t reg, uint8_t *data, uint8_t len) {
    esp_err_t ret;
    ret = i2c_master_write_read_device(CONFIG_I2C_MASTER_PORT_NUM, address, &reg, 1, data, len, CONFIG_I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return (bool)(ret == ESP_OK);
}