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
#include "math.h"
#define TAG "PRESSURE_DRIVER"

pressure_driver_status_t pressure_driver_init(pressure_driver_struct_t *pressure_driver) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }
    ads1115_mode_t mode;
    ads1115_data_rate_t rate;
    ads1115_set_mode(pressure_driver->ads1115, ADS1115_MODE_CONTINUOUS);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_data_rate(pressure_driver->ads1115, ADS1115_DATA_RATE_860);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_set_gain(pressure_driver->ads1115, ADS1115_GAIN_4V096);
    vTaskDelay(pdMS_TO_TICKS(50));
    ads1115_get_mode(pressure_driver->ads1115, &mode);
    printf("MODE = %d", (bool)mode);
    ads1115_get_data_rate(pressure_driver->ads1115, &rate);
    printf("RATE = %d", (uint8_t)rate);
    //vTaskDelay(pdMS_TO_TICKS(2500));
    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_zero_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].calibr_cfg.voltage_zero = voltage;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_1_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].calibr_cfg.voltage_1 = voltage;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_set_1_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float pressure) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    pressure_driver->sensors[sensor].calibr_cfg.pressure_1 = pressure;

    return PRESSURE_DRIVER_OK;
}

pressure_driver_status_t pressure_driver_read_voltage(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor, float *voltage) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }
    int16_t raw;

    
        ads1115_set_input_mux(pressure_driver->ads1115, pressure_driver->sensors[sensor].adc_pin);
        vTaskDelay(pdMS_TO_TICKS(10));
    int mux_num = pressure_driver->sensors[sensor].adc_pin;
    vTaskDelay(pdMS_TO_TICKS(10));
    int16_t dummy;
    ads1115_get_value(pressure_driver->ads1115, &dummy);

    // 4. Czytaj właściwy wynik
    ads1115_get_value(pressure_driver->ads1115, &raw);
    *voltage = ads1115_gain_values[ADS1115_GAIN_4V096] / ADS1115_MAX_VALUE * raw;
    return PRESSURE_DRIVER_OK;
}

float pressure_driver_read_pressure(pressure_driver_struct_t *pressure_driver, pressure_driver_sensor_t sensor) {
    if (pressure_driver == NULL) {
        return PRESSURE_DRIVER_FAIL;
    }

    float voltage;
    float pressure;
    pressure_driver_read_voltage(pressure_driver, sensor, &voltage);
    pressure = (voltage - pressure_driver->sensors[sensor].calibr_cfg.voltage_zero) * (pressure_driver->sensors[sensor].calibr_cfg.pressure_1) / (pressure_driver->sensors[sensor].calibr_cfg.voltage_1 - pressure_driver->sensors[sensor].calibr_cfg.voltage_zero);

    return pressure;
}

pressure_driver_status_t pressure_driver_read_pressures(pressure_driver_struct_t *pressure_driver, float *pressures, float *voltages) {
    if (pressure_driver == NULL || pressures == NULL || voltages == NULL) {
        ESP_LOGE(TAG, "Invalid argument in pressure_driver_read_pressures");
        return PRESSURE_DRIVER_FAIL;
    }

    for (int i = 0; i < PRESSURE_DRIVER_SENSOR_COUNT; i++) {
        // 1. Odczyt napięcia
        float v_raw;
        pressure_driver_status_t status = pressure_driver_read_voltage(pressure_driver, i, &v_raw);
        if (status != PRESSURE_DRIVER_OK) {
            ESP_LOGE(TAG, "Failed to read voltage for sensor %d", i);
            return status;
        }

        // Zapisujemy napięcie do tablicy wyjściowej
        voltages[i] = v_raw;

        // 2. Pobieramy punkty kalibracji dla czytelności wzoru
        float v0 = pressure_driver->sensors[i].calibr_cfg.voltage_zero;
        float v1 = pressure_driver->sensors[i].calibr_cfg.voltage_1;
        float p1 = pressure_driver->sensors[i].calibr_cfg.pressure_1;

        // 3. Obliczanie ciśnienia (Wzór prostej przechodzącej przez (v0, 0) i (v1, p1))
        // Unikamy dzielenia przez zero, jeśli kalibracja nie została wykonana
        if (fabsf(v1 - v0) < 0.0001f) {
            pressures[i] = 0.0f;
        } else {
            pressures[i] = (v_raw - v0) * (p1 / (v1 - v0));
        }

        // Jeśli ciśnienie wyjdzie lekko ujemne (szum przy 0 bar), utnij do zera
        if (pressures[i] < 0) pressures[i] = 0;
    }
    return PRESSURE_DRIVER_OK;
}