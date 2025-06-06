///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 12.02.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the pressure sensor utility. It is used to read the pressure
/// from the sensor and convert it to the physical value. The sensor is connected to the ADS1115
/// Analog-to-Digital Converter.
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_PRESSURE_DRIVER_H_
#define PWRINSPACE_PRESSURE_DRIVER_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "ads1115.h"

#define PRESSURE_DRIVER_SENSOR_COUNT 4

#define PRESSURE_DRIVER_DEFAULT_MIN_PRESSURE 0.0f
#define PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE 350.0f

#define PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE 0.33f
#define PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE 2.80f

#define PRESSURE_DRIVER_TANWA_CONFIG(X)                         \
  {                                                             \
    .ads1115 = X,                                               \
    .sensors = {                                                \
      {                                                         \
        .sensor = PRESSURE_DRIVER_SENSOR_1,                     \
        .adc_pin = PRESSURE_DRIVER_SENSOR_1_ADC_PIN,            \
        .pressure_min = PRESSURE_DRIVER_DEFAULT_MIN_PRESSURE,   \
        .pressure_max = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,   \
        .voltage_min = PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE,     \
        .voltage_max = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,     \
      },                                                        \
      {                                                         \
        .sensor = PRESSURE_DRIVER_SENSOR_2,                     \
        .adc_pin = PRESSURE_DRIVER_SENSOR_2_ADC_PIN,            \
        .pressure_min = PRESSURE_DRIVER_DEFAULT_MIN_PRESSURE,   \
        .pressure_max = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,   \
        .voltage_min = PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE,     \
        .voltage_max = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,     \
      },                                                        \
      {                                                         \
        .sensor = PRESSURE_DRIVER_SENSOR_3,                     \
        .adc_pin = PRESSURE_DRIVER_SENSOR_3_ADC_PIN,            \
        .pressure_min = PRESSURE_DRIVER_DEFAULT_MIN_PRESSURE,   \
        .pressure_max = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,   \
        .voltage_min = PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE,     \
        .voltage_max = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,     \
      },                                                        \
      {                                                         \
        .sensor = PRESSURE_DRIVER_SENSOR_4,                     \
        .adc_pin = PRESSURE_DRIVER_SENSOR_4_ADC_PIN,            \
        .pressure_min = PRESSURE_DRIVER_DEFAULT_MIN_PRESSURE,   \
        .pressure_max = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,   \
        .voltage_min = PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE,     \
        .voltage_max = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,     \
      },                                                        \
    }                                                           \
  }

typedef enum {
    PRESSURE_DRIVER_SENSOR_1 = 0,
    PRESSURE_DRIVER_SENSOR_2,
    PRESSURE_DRIVER_SENSOR_3,
    PRESSURE_DRIVER_SENSOR_4,
} pressure_driver_sensor_t;

typedef enum {
    PRESSURE_DRIVER_SENSOR_1_ADC_PIN = ADS1115_MUX_0_GND,
    PRESSURE_DRIVER_SENSOR_2_ADC_PIN = ADS1115_MUX_1_GND,
    PRESSURE_DRIVER_SENSOR_3_ADC_PIN = ADS1115_MUX_2_GND,
    PRESSURE_DRIVER_SENSOR_4_ADC_PIN = ADS1115_MUX_3_GND,
} pressure_driver_sensor_adc_pin_t;

typedef enum {
    PRESSURE_DRIVER_OK = 0,
    PRESSURE_DRIVER_FAIL = 1,
    PRESSURE_DRIVER_READ_ERR = 2,
} pressure_driver_status_t;

typedef struct {
    pressure_driver_sensor_t sensor;
    pressure_driver_sensor_adc_pin_t adc_pin;
    float pressure_min;
    float pressure_max;
    float voltage_min;
    float voltage_max;
} pressure_sensor_struct_t;

typedef struct {
    ads1115_struct_t *ads1115;
    pressure_sensor_struct_t sensors[PRESSURE_DRIVER_SENSOR_COUNT];
} pressure_driver_struct_t;

pressure_driver_status_t pressure_driver_init(pressure_driver_struct_t *pressure_driver);

pressure_driver_status_t pressure_driver_set_min_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float pressure);

pressure_driver_status_t pressure_driver_set_max_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float pressure);

pressure_driver_status_t pressure_driver_set_min_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage);

pressure_driver_status_t pressure_driver_set_max_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage);

pressure_driver_status_t pressure_driver_read_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float *voltage);

pressure_driver_status_t pressure_driver_read_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float *pressure);

#endif /* PWRINSPACE_PRESSURE_DRIVER_H_ */