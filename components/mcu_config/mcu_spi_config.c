///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 15.05.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//

#include "mcu_spi_config.h"

#include "esp_log.h"

#define TAG "MCU_SPI"

static mcu_spi_config_t spi_sd_config = MCU_SPI_SD_CONFIG();
static mcu_spi_config_t spi_tc_config = MCU_SPI_TC_CONFIG();
SemaphoreHandle_t mutex_spi;
SemaphoreHandle_t mutex_spi_tc;

esp_err_t mcu_spi_init(void) {
  esp_err_t ret = ESP_OK;
  if (spi_sd_config.spi_init_flag && spi_tc_config.spi_init_flag) {
    return ESP_OK;
  }

  ret = spi_bus_initialize(spi_sd_config.host_id, &spi_sd_config.bus_config,
                           SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SD SPI bus init failed: %s", esp_err_to_name(ret));
    return ret;
  }
  spi_sd_config.spi_init_flag = true;

  ret = spi_bus_initialize(spi_tc_config.host_id, &spi_tc_config.bus_config,
                           SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "TC SPI bus init failed: %s", esp_err_to_name(ret));
    return ret;
  }
  spi_tc_config.spi_init_flag = true;

  mutex_spi = xSemaphoreCreateMutex();
  mutex_spi_tc = xSemaphoreCreateMutex();
  return ESP_OK;
}

esp_err_t mcu_spi_deinit(void) {
  esp_err_t ret = ESP_OK;
  if (spi_tc_config.spi_init_flag) {
    ret = spi_bus_free(spi_tc_config.host_id);
    ESP_ERROR_CHECK(ret);
    spi_tc_config.spi_init_flag = false;
  }
  if (spi_sd_config.spi_init_flag) {
    ret = spi_bus_free(spi_sd_config.host_id);
    ESP_ERROR_CHECK(ret);
    spi_sd_config.spi_init_flag = false;
  }
  return ret;
}
