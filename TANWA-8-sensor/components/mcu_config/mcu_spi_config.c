///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 15.05.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//

#include "mcu_spi_config.h"

#include "mcu_gpio_config.h"
#include "max31856.h"
#include "board_config.h"
#include "esp_log.h"

#define TAG "MCU_SPI"

static mcu_spi_config_t spi_config = MCU_SPI_DEFAULT_CONFIG();
SemaphoreHandle_t mutex_spi;

esp_err_t mcu_spi_init(void) {
    esp_err_t ret = ESP_OK;
    if (spi_config.spi_init_flag) {
      return ESP_OK;
    }

    ret = spi_bus_initialize(spi_config.host_id, &spi_config.bus_config, SDSPI_DEFAULT_DMA);
    ESP_ERROR_CHECK(ret);

    ret=spi_bus_add_device(spi_config.host_id, &spi_config.dev_config, &config.thermocouple[0].spi);
    ret=spi_bus_add_device(spi_config.host_id, &spi_config.dev_config, &config.thermocouple[1].spi);
    ret=spi_bus_add_device(spi_config.host_id, &spi_config.dev_config, &config.thermocouple[2].spi);
    ESP_ERROR_CHECK(ret);
    mutex_spi = xSemaphoreCreateMutex();
    spi_config.spi_init_flag = true;
    return ret;
}

esp_err_t mcu_spi_deinit(void) {
    esp_err_t ret = ESP_OK;
    if (!spi_config.spi_init_flag) {
      return ESP_OK;
    }
    ret = spi_bus_remove_device(spi_config.spi_handle);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_free(spi_config.host_id);
    ESP_ERROR_CHECK(ret);
    spi_config.spi_init_flag = false;
    return ret;
}