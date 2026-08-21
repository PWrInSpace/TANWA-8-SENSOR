///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 15.05.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains the configuration of the SPI of the MCU.
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_MCU_SPI_CONFIG_H_
#define PWRINSPACE_MCU_SPI_CONFIG_H_

#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pinout.h"
#include "rom/gpio.h"
#include "sdkconfig.h"
#include "soc/gpio_struct.h"

#define MCU_SPI_SD_HOST SPI2_HOST
#define MCU_SPI_TC_HOST SPI3_HOST

#define MCU_SPI_SD_CONFIG()                                                    \
  {                                                                            \
    .host_id = MCU_SPI_SD_HOST,                                                \
    .bus_config = {.miso_io_num = SPI_SD_MISO,                                 \
                   .mosi_io_num = SPI_SD_MOSI,                                 \
                   .sclk_io_num = SPI_SD_SCK,                                  \
                   .quadwp_io_num = -1,                                        \
                   .quadhd_io_num = -1,                                        \
                   .max_transfer_sz = 4000},                                   \
    .dev_config = {.clock_speed_hz = 1000000,                                  \
                   .mode = 0,                                                  \
                   .spics_io_num = -1,                                         \
                   .queue_size = 1,                                            \
                   .flags = 0,                                                 \
                   .pre_cb = NULL},                                            \
    .spi_init_flag = false,                                                    \
  }

#define MCU_SPI_TC_CONFIG()                                                    \
  {                                                                            \
    .host_id = MCU_SPI_TC_HOST,                                                \
    .bus_config = {.miso_io_num = SPI_TC_MISO,                                 \
                   .mosi_io_num = SPI_TC_MOSI,                                 \
                   .sclk_io_num = SPI_TC_SCK,                                  \
                   .quadwp_io_num = -1,                                        \
                   .quadhd_io_num = -1,                                        \
                   .max_transfer_sz = 4000},                                   \
    .dev_config = {.clock_speed_hz = 1000000,                                  \
                   .mode = 1,                                                  \
                   .spics_io_num = -1,                                         \
                   .queue_size = 1,                                            \
                   .flags = 0,                                                 \
                   .pre_cb = NULL},                                            \
    .spi_init_flag = false,                                                    \
  }

typedef struct {
  spi_host_device_t host_id;
  spi_device_handle_t spi_handle;
  spi_bus_config_t bus_config;
  spi_device_interface_config_t dev_config;
  bool spi_init_flag;
} mcu_spi_config_t;

extern SemaphoreHandle_t mutex_spi;
extern SemaphoreHandle_t mutex_spi_tc;

/**
 * \brief Initiates the SPI buses (SD on SPI2, thermocouples on SPI3)
 * \return ESP_OK on success, ESP_FAIL otherwise
 * \note This function will only initiate SPI peripherals,
 *      it will not configure the GPIO for CS pins.
 */
esp_err_t mcu_spi_init(void);

/**
 * \brief Deinitializes the SPI buses
 * \return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t mcu_spi_deinit(void);

#endif /* PWRINSPACE_MCU_SPI_CONFIG_H_ */
