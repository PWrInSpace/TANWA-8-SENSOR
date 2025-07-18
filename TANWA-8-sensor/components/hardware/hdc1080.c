#include "hdc1080.h"
#include "esp_log.h"
static esp_err_t write_reg_8b(HDC1080_dev *hd1080, const uint8_t reg, uint8_t val) {
    bool ret = true;
    ret = hd1080->_i2c_write(hd1080->i2c_address, reg, &val, 1);
    return ret ? ESP_OK : ESP_FAIL;
}

static esp_err_t write_reg_16b(HDC1080_dev *hd1080, const uint8_t reg, uint16_t val) {
    bool ret = true;
    uint8_t data[2] = {val >> 8, val & 0xFF};
    ret = hd1080->_i2c_write(hd1080->i2c_address, reg, data, 2);
    return ret ? ESP_OK : ESP_FAIL;
}

static esp_err_t read_reg_16b(HDC1080_dev *hd1080, const uint8_t reg, uint16_t *val) {
    bool ret = true;
    ret = hd1080->_i2c_write(hd1080->i2c_address, reg,0,0);
    uint8_t data[2];
    ret = hd1080->_i2c_read(hd1080->i2c_address, reg, data, 2);
    *val = (data[0] << 8) | data[1];
    return ret ? ESP_OK : ESP_FAIL;
}

static esp_err_t read_reg_8b(HDC1080_dev *hd1080, const uint8_t reg, uint8_t *val) {
    bool ret = true;
    ret = hd1080->_i2c_write(hd1080->i2c_address, reg,0,0);
    uint8_t data = 0;
    ret = hd1080->_i2c_read(hd1080->i2c_address, reg, &data, 1);
    *val = data;
    return ret ? ESP_OK : ESP_FAIL;
}

static void read_config_bits(HDC1080_dev *hd1080, const uint8_t mask, const uint8_t offset, uint16_t * val)
{
    uint16_t config_reg;
    if(read_reg_16b(hd1080, HDC1080_CONFIG_REG, &config_reg)!=ESP_OK)
    {
       // ESP_LOGE("HDC1080", "FAILED TO READ CONFIG REGISTERS");
    }

   // ESP_LOGD("HDC1080", "Got config value: 0x%04x", config_reg);
    *val = (config_reg>>offset) &mask;

}

esp_err_t write_config_bits(HDC1080_dev *hd1080, const uint8_t mask, const uint8_t offset, uint16_t val)
{
    esp_err_t ret;
    uint16_t config_old = 0;
    read_reg_16b(hd1080, HDC1080_CONFIG_REG, &config_old);
    ret = write_reg_16b(hd1080, HDC1080_CONFIG_REG, (config_old & ~(mask << offset)) | (val << offset));
    if (ret != ESP_OK) {
       // ESP_LOGE("HDC1080", "Could not write config register");
        return ret;
    }

    return ESP_OK;

}

void hd1080_init(HDC1080_dev * dev)
{
    uint16_t val;
    uint8_t v = 0;
    write_config_bits(dev,1,MODE_OFFSET,HDC1080_ACQUISITION_HUMIDITY_AND_TEMPERATURE);
    write_config_bits(dev,1,TEMP_RES_OFFSET,HDC1080_TEMPERATURE_RESOLUTION_14BIT);
    read_reg_16b(dev, HDC1080_CONFIG_REG, &val);
    printf("HD1080 Register =0x%04x", val); 
}

void hdc1080_read_temperature(HDC1080_dev *dev) {
    uint16_t val[2];    
    dev->_i2c_write(dev->i2c_address, HDC1080_TEMPERATURE_REG,0,0);
    //usleep(6350);
    printf("Temperature Register = 0x%04x\n", val[0]);
    read_reg_16b(dev, HDC1080_TEMPERATURE_REG, &val[0]);
    printf("TEMPERATURE Register =0x%d", val[0]); 
    printf("HUMIDITY Register =0x%d", val[1]); 
    // Calculate temperature in °C
    float temp = ((float)val[0] / 65536.0) * 165.0 - 40.0;
    printf("Temperature (°C) = %.2f\n", temp);
}