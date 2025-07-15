///===-----------------------------------------------------------------------------------------===//
///
/// Copyright (c) PWr in Space. All rights reserved.
/// Created: 12.02.2024 by Michał Kos
///
///===-----------------------------------------------------------------------------------------===//

#include "pressure_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#define TAG "PRESSURE_DRIVER"
pressure_driver_status_t pressure_driver_init(pressure_driver_struct_t *pressure_driver) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    ads1115_set_mode(pressure_driver->ads1115, ADS1115_MODE_CONTINUOUS);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_data_rate(pressure_driver->ads1115, ADS1115_DATA_RATE_475);
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
    ads1115_get_value(pressure_driver->ads1115, &raw);
    vTaskDelay(pdMS_TO_TICKS(3));
    ads1115_set_input_mux(pressure_driver->ads1115, pressure_driver->sensors[sensor].adc_pin);    
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

pressure_driver_status_t pressure_driver_read_pressures(pressure_driver_struct_t *pressure_driver, float *pressure) {
    if (pressure_driver == NULL || pressure == NULL) {
        ESP_LOGE(TAG, "Invalid argument in pressure_driver_read_pressures");
        return PRESSURE_DRIVER_FAIL;
    }

    float voltage[PRESSURE_DRIVER_SENSOR_COUNT];
    for (int i = 0; i < PRESSURE_DRIVER_SENSOR_COUNT; i++) {
        // Read voltage for the current sensor
        pressure_driver_status_t status = pressure_driver_read_voltage(pressure_driver, i, &voltage[i]);
        if (status != PRESSURE_DRIVER_OK) {
            ESP_LOGE(TAG, "Failed to read voltage for sensor %d", i);
            return status;
        }

        // Calculate pressure for the current sensor
        pressure[i] = (voltage[i] - pressure_driver->sensors[i].voltage_min) * 
                      (pressure_driver->sensors[i].pressure_max - pressure_driver->sensors[i].pressure_min) / 
                      (pressure_driver->sensors[i].voltage_max - pressure_driver->sensors[i].voltage_min) + 
                      pressure_driver->sensors[i].pressure_min;
        ESP_LOGI(TAG, "Sensor %d, voltage: %.3f V, pressure: %.3f", i, voltage[i], pressure[i]);
    }
    return PRESSURE_DRIVER_OK;
}