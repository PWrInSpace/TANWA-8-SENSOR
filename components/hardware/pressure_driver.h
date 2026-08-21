///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 12.02.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//
///
/// \file
/// This file contains declaration of the pressure sensor utility. It is used to
/// read the pressure from the sensor and convert it to the physical value. The
/// sensor is connected to the ADS1115 Analog-to-Digital Converter.
///===-----------------------------------------------------------------------------------------===//

#ifndef PWRINSPACE_PRESSURE_DRIVER_H_
#define PWRINSPACE_PRESSURE_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ads1115.h"
#include "max31856.h"

#define PRESSURE_DRIVER_SENSOR_COUNT 4

#define PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE 350.0f

// 3.6k
//#define PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE 0.3597f
//#define PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE 3.237f

// 3.3k
#define PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE 0.3759f
#define PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE 3.3835f

//#define PRESSURE_DRIVER_DEFAULT_MIN_VOLTAGE 0.495f
//#define PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE 4.4554f

#define PRESSURE_DRIVER_TANWA_CONFIG1(X)                                       \
  {                                                                            \
    .ads1115 = X, .sensors = {                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_1,                                     \
       .adc_pin = PRESSURE_DRIVER1_SENSOR_1_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_2,                                     \
       .adc_pin = PRESSURE_DRIVER1_SENSOR_2_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_3,                                     \
       .adc_pin = PRESSURE_DRIVER1_SENSOR_3_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_4,                                     \
       .adc_pin = PRESSURE_DRIVER1_SENSOR_4_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
    }                                                                          \
  }

#define PRESSURE_DRIVER_TANWA_CONFIG2(X)                                       \
  {                                                                            \
    .ads1115 = X, .sensors = {                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_1,                                     \
       .adc_pin = PRESSURE_DRIVER2_SENSOR_1_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_2,                                     \
       .adc_pin = PRESSURE_DRIVER2_SENSOR_2_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_3,                                     \
       .adc_pin = PRESSURE_DRIVER2_SENSOR_3_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
      {.sensor = PRESSURE_DRIVER_SENSOR_4,                                     \
       .adc_pin = PRESSURE_DRIVER2_SENSOR_4_ADC_PIN,                           \
       .calibr_cfg =                                                           \
           {                                                                   \
               .voltage_zero = PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE,           \
               .pressure_1 = PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE,             \
               .voltage_1 = PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE,               \
           }},                                                                 \
    }                                                                          \
  }

typedef enum {
  PRESSURE_DRIVER_SENSOR_1 = 0,
  PRESSURE_DRIVER_SENSOR_2,
  PRESSURE_DRIVER_SENSOR_3,
  PRESSURE_DRIVER_SENSOR_4,
} pressure_driver_sensor_t;

typedef enum {
  PRESSURE_DRIVER1_SENSOR_1_ADC_PIN = ADS1115_MUX_3_GND, // 7 //N2_ZF
  PRESSURE_DRIVER1_SENSOR_2_ADC_PIN = ADS1115_MUX_0_GND, // 6 //DRD_N2
  PRESSURE_DRIVER1_SENSOR_3_ADC_PIN = ADS1115_MUX_1_GND, // 5 //DRD_N2O
  PRESSURE_DRIVER1_SENSOR_4_ADC_PIN = ADS1115_MUX_2_GND, // 4 //BLANK
  PRESSURE_DRIVER2_SENSOR_1_ADC_PIN = ADS1115_MUX_3_GND, // CUTOFF N2O
  PRESSURE_DRIVER2_SENSOR_2_ADC_PIN = ADS1115_MUX_0_GND, // N2_ZR
  PRESSURE_DRIVER2_SENSOR_3_ADC_PIN = ADS1115_MUX_1_GND, // N2 PR
  PRESSURE_DRIVER2_SENSOR_4_ADC_PIN = ADS1115_MUX_2_GND, // N2O ZA FILLEM
} pressure_driver_sensor_adc_pin_t;

typedef struct {
  float voltage_zero;
  float voltage_1;
  float pressure_1;
} pressure_sensor_calibration_config_t;

typedef enum {
  PRESSURE_DRIVER_OK = 0,
  PRESSURE_DRIVER_FAIL = 1,
  PRESSURE_DRIVER_READ_ERR = 2,
} pressure_driver_status_t;

typedef struct {
  pressure_driver_sensor_t sensor;
  pressure_driver_sensor_adc_pin_t adc_pin;
  pressure_sensor_calibration_config_t calibr_cfg;
} pressure_sensor_struct_t;

typedef struct {
  ads1115_struct_t *ads1115;
  pressure_sensor_struct_t sensors[PRESSURE_DRIVER_SENSOR_COUNT];
} pressure_driver_struct_t;

pressure_driver_status_t
pressure_driver_init(pressure_driver_struct_t *pressure_driver);

pressure_driver_status_t
pressure_driver_set_zero_voltage(pressure_driver_struct_t *pressure_driver,
                                 pressure_driver_sensor_t sensor,
                                 float voltage);

pressure_driver_status_t
pressure_driver_set_1_voltage(pressure_driver_struct_t *pressure_driver,
                              pressure_driver_sensor_t sensor, float voltage);

pressure_driver_status_t
pressure_driver_set_1_pressure(pressure_driver_struct_t *pressure_driver,
                               pressure_driver_sensor_t sensor, float pressure);

pressure_driver_status_t
pressure_driver_read_voltage(pressure_driver_struct_t *pressure_driver,
                             pressure_driver_sensor_t sensor, float *voltage);

float pressure_driver_read_pressure(pressure_driver_struct_t *pressure_driver,
                                    pressure_driver_sensor_t sensor);
pressure_driver_status_t
pressure_driver_read_pressures(pressure_driver_struct_t *pressure_driver,
                               float *pressures, float *voltages);

#endif /* PWRINSPACE_PRESSURE_DRIVER_H_ */