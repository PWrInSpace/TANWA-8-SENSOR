//Skopiowany bez zmian z: Tanwa-7-THERMO

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include "esp_err.h"

#include "max31856.h"
#include "pressure_sensor.h"

typedef struct {
    Pressure_Sensor_t pressure_sensor;
    max31856_cfg termocouple;
    uint8_t is_initialized;
} hardware_config_t;

extern hardware_config_t hardware_config;

esp_err_t hardware_config_init(void);

#endif // HARDWARE_CONFIG_H