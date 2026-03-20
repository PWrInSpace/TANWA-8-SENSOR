///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 27.01.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the system console configuration, including initialization
/// and available commands for debugging/testing purposes.
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_BOARD_CONFIG_H
#define PWRINSPACE_BOARD_CONFIG_H

#include "led_driver.h"
#include "esp_err.h"
#include "ads1115.h"
#include "max31856.h"
#include "mcp_driver.h"
#include "pressure_driver.h"
#include "tmp1075.h"
#include "hdc1080.h"

//#define TMP1075_QUANTITY 1
#define MAX31856_QUANTITY 3
#define ADS1115_QUANTITY 2

typedef struct {

    char board_name[32];
    led_struct_t status_led;
    tmp1075_struct_t tmp1075;
    max31856_cfg thermocouple[MAX31856_QUANTITY];
    ads1115_struct_t ads1115[ADS1115_QUANTITY];
    pressure_driver_struct_t pressure_driver[ADS1115_QUANTITY];
    mcp342x_driver_t mcp342x;
    HDC1080_dev hdc;

} board_config_t;

extern board_config_t config;

esp_err_t board_config_init(void);

#endif /* PWRINSPACE_BOARD_CONFIG_H */


// RTR -> TEN SAM ID U MASTER I SLAVE BO RTR PRZEGRYWA WALKE O MIEJSCE!