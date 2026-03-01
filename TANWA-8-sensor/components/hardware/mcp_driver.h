#ifndef PWRINSPACE_MCP_DRIVER_H
#define PWRINSPACE_MCP_DRIVER_H

#include "math.h"
#include "i2cdev.h"
#include "mcu_i2c_config.h"
#include "esp_log.h"
#include "esp_err.h"
#include "mcp342x.h"

#define MCP_I2C_SCL_PIN SCL_GPIO
#define MCP_I2C_SDA_PIN SDA_GPIO
#define MCP_I2C_PORT CONFIG_I2C_MASTER_PORT_NUM
#define MCP_ADDR 0x68

esp_err_t mcp_driver_init(void);

esp_err_t mcp_driver_read_voltage(mcp342x_channel_t channel, float *out_voltage);

esp_err_t mcp_driver_read_PT100_temp(mcp342x_channel_t channel, float *out_temp);

#endif //PWRINSPACE_MCP_DRIVER_H