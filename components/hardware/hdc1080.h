#pragma once
#include "driver/gpio.h"
#include <math.h>
#define HDC1080_TEMPERATURE_REG 0x00 /* TEMPERATURE MEASUREMENT OUTPUT */
#define HDC1080_HUMIDITY_REG 0x01    /* RELATIVE HUMIDITY MEASUREMENT OUTPUT */
#define HDC1080_CONFIG_REG 0x02      /* HDC1080 CONFIGURATION DATA */
#define HDC1080_SERIALID2_REG 0xFB   /* FIRST 2 BYTES OF SERIAL ID */
#define HDC1080_SERIALID1_REG 0xFC   /* MID 2 BYTES OF THE SERIAL ID */
#define HDC1080_SERIALID0_REG 0xFD   /* LAST BYTE BIT OF THE SERIAL ID */
#define HDC1080_MANUFACTURER_ID_REG 0xFE /* ID OF TEXAS INSTRUMENTS */
#define HDC1080_DEVICE_ID_REG 0xFF       /* REGISTER OF THE DEVICE ID */
#define HDC1080_DEVICE_ID 0x1050         /* HDC1080 UNIQUE ID */
#define HDC1080_MANUFACTURER_ID 0x5449   /* TI MANUFACTURER ID */
#define HDC1080_I2C_ADDRESS 0x40         /* I2C ADDRESS OF THE HDC1080 */

#define HDC1080_ACQUISITION_HUMIDITY_AND_TEMPERATURE 0x01
#define HDC1080_ACQUISITION_HUMIDITY_OR_TEMPERATURE 0x00
#define HDC1080_TEMPERATURE_RESOLUTION_11BIT 0x01
#define HDC1080_TEMPERATURE_RESOLUTION_14BIT 0x00
#define HDC1080_HUMIDITY_RESOLUTION_8BIT 0x02
#define HDC1080_HUMIDITY_RESOLUTION_11BIT 0x01
#define HDC1080_HUMIDITY_RESOLUTION_14BIT 0x00

#define HDC1080_HEATER_ENABLED 0x01
#define HDC1080_HEATER_DISABLED 0x00
#define HDC1080_BATTERY_STATUS_OK 0x00
#define HDC1080_BATTERY_STATUS_LOW 0x01
#define HDC1080_ERR_ID 0xFF
#define HDC1080_CONVERTING 0xFE
#define HDC1080_CONVERSION_WAIT_PERIOD (500000) /* CONVERSION WAIT PERIOD */

#define MODE_OFFSET 0x0C
#define TEMP_RES_OFFSET 0x0A
#define HUMIDITY_RES_OFFSET 0x08

typedef bool (*hdc1080_I2C_write)(uint8_t address, uint8_t reg, uint8_t *data,
                                  uint8_t len);
typedef bool (*hdc1080_I2C_read)(uint8_t address, uint8_t reg, uint8_t *data,
                                 uint8_t len);

typedef struct {
  uint8_t i2c_address;
  uint8_t humidity_percentage;
  uint8_t temperature;
  hdc1080_I2C_write _i2c_write;
  hdc1080_I2C_read _i2c_read;
} HDC1080_dev;

void hd1080_init(HDC1080_dev *dev);
static esp_err_t write_reg_8b(HDC1080_dev *hd1080, const uint8_t reg,
                              uint8_t val);

static esp_err_t write_reg_16b(HDC1080_dev *hd1080, const uint8_t reg,
                               uint16_t val);

static esp_err_t read_reg_16b(HDC1080_dev *hd1080, const uint8_t reg,
                              uint16_t *val);

static void read_config_bits(HDC1080_dev *hd1080, const uint8_t mask,
                             const uint8_t offset, uint16_t *val);

esp_err_t write_config_bits(HDC1080_dev *hd1080, const uint8_t mask,
                            const uint8_t offset, uint16_t val);

void hdc1080_read_temperature(HDC1080_dev *dev);