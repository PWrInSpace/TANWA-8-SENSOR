///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 12.02.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//

#include "pressure_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

pressure_driver_status_t pressure_driver_init(pressure_driver_struct_t *pressure_driver) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    ads1115_set_mode(pressure_driver->ads1115, ADS1115_MODE_CONTINUOUS);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_data_rate(pressure_driver->ads1115, ADS1115_DATA_RATE_250);
    vTaskDelay(pdMS_TO_TICKS(50));

    // PRESSURE SENSOR 1
    ads1115_set_input_mux(pressure_driver->ads1115, PRESSURE_DRIVER_SENSOR_1_ADC_PIN);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_gain(pressure_driver->ads1115, ADS1115_GAIN_4V096);
    vTaskDelay(pdMS_TO_TICKS(50));

    // PRESSURE SENSOR 2
    ads1115_set_input_mux(pressure_driver->ads1115, PRESSURE_DRIVER_SENSOR_2_ADC_PIN);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_gain(pressure_driver->ads1115, ADS1115_GAIN_4V096);
    vTaskDelay(pdMS_TO_TICKS(50));

    // PRESSURE SENSOR 3
    ads1115_set_input_mux(pressure_driver->ads1115, PRESSURE_DRIVER_SENSOR_3_ADC_PIN);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_gain(pressure_driver->ads1115, ADS1115_GAIN_4V096);
    vTaskDelay(pdMS_TO_TICKS(50));

    // PRESSURE SENSOR 4
    ads1115_set_input_mux(pressure_driver->ads1115, PRESSURE_DRIVER_SENSOR_4_ADC_PIN);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_gain(pressure_driver->ads1115, ADS1115_GAIN_4V096);
    vTaskDelay(pdMS_TO_TICKS(50));

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_min_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float pressure) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].pressure_min = pressure;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_max_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float pressure) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].pressure_max = pressure;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_min_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].voltage_min = voltage;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_max_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].voltage_max = voltage;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_read_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float *voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    int16_t raw;
    vTaskDelay(pdMS_TO_TICKS(5));
    ads1115_set_input_mux(pressure_driver->ads1115, pressure_driver->sensors[sensor].adc_pin);
    vTaskDelay(pdMS_TO_TICKS(5));
    ads1115_get_value(pressure_driver->ads1115, &raw);
    *voltage = ads1115_gain_values[ADS1115_GAIN_4V096] / ADS1115_MAX_VALUE * raw;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_read_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float *pressure) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    float voltage;
    pressure_driver_read_voltage(pressure_driver, sensor, &voltage);
    *pressure = (voltage - pressure_driver->sensors[sensor].voltage_min) * (pressure_driver->sensors[sensor].pressure_max - pressure_driver->sensors[sensor].pressure_min) / (pressure_driver->sensors[sensor].voltage_max - pressure_driver->sensors[sensor].voltage_min) + pressure_driver->sensors[sensor].pressure_min;

    return PRESSURE_DRIVER_OK;
}