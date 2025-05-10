//Skopiowany bez zmian z: Tanwa-7-THERMO

#include "hardware_config.h"

#include "driver/gpio.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#include "spi_config.h"

#include "esp_log.h"
#include "esp_err.h"


#include "max31856.h"

//TANWWA-7-COM pressure_driver.h ads1115.h

#define  ADC_ATTEN_VOLTAGE ADC_ATTEN_DB_11

static const char *TAG = "hardware_config";

hardware_config_t hardware_config = {
  .pressure_sensor = {
    .adc_cali_handle = NULL,
    .adc_handle = NULL,
    .adc_raw = 0,
    .cali_enable = false,
    .voltage = 0.0,
  },
  .termocouple = {
    .coldjunction_c = 0.0,
    .coldjunction_f = 0.0,
    .cs_pin = 0,
    .fault = 0,
    .spi = NULL,
    .termocouple_c = 0.0,
    .termocouple_f = 0.0,
  }
};



esp_err_t hardware_config_init(void) {
  ESP_LOGI(TAG, "Initializing hardware configuration...");

  SPI_cfg spi_cfg = {
    .spi_bus = {
      .mosi_io_num = CONFIG_SPI_MOSI,
      .miso_io_num = CONFIG_SPI_MISO,
      .sclk_io_num = CONFIG_SPI_SCK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0,
    },
    .spi_host = VSPI_HOST,
  };


  if(adc_oneshot_new_unit(&adc_init_config_1, hardware_config.pressure_sensor.adc_handle) != ESP_OK){
    ESP_LOGE(TAG, "ADC1 initialization failed");
    return ESP_FAIL;
  }

  hardware_config.pressure_sensor.cali_enable = adc_calibration_init();
  // ### VOLTAGE INIT ###
  hardware_config.pressure_sensor.voltage = 0;

  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_VOLTAGE,
  };
  if(adc_oneshot_config_channel(*hardware_config.pressure_sensor.adc_handle, hardware_config.pressure_sensor.adc_channel, &config) != ESP_OK){
    ESP_LOGE(TAG, "Voltage measurement initialization failed");
    return ESP_FAIL;
  }

  pressure_sensor_init(&hardware_config.pressure_sensor);

  ESP_LOGI(TAG, "termocouple initialization...");

  max31856_init(&hardware_config.termocouple, CONFIG_TERMO_CS); 

  ESP_LOGI(TAG, "termocouple set type...");
  termocouple_set_type(&hardware_config.termocouple, MAX31856_TCTYPE_K);
  ESP_LOGI(TAG, "termocouple read fault...");
  termocouple_read_fault(&hardware_config.termocouple, true);


  // ### SET INITIALIZED FLAG ###
  hardware_config.is_initialized = true;
  return ESP_OK;
}