#include "pressure_driver.h"

// DATA(name, type, default_value)
// DATA_ARRAY(name, type, array_size, default_value)

// obecnie wspierane typy: int32_t, uint8_t, float, double, char, char[]
// możliwość rozszerzenia wspieranych typów w pliku flash.c (należy na samym dole dodać parser i zaktualizować funkcje update_field)

#define CONFIG_FIELDS                                                                           \
    DATA(pressure_driver_0_sensor_0_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_0_sensor_0_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_0_sensor_0_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_0_sensor_1_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_0_sensor_1_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_0_sensor_1_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_0_sensor_2_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_0_sensor_2_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_0_sensor_2_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_0_sensor_3_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_0_sensor_3_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_0_sensor_3_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_1_sensor_0_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_1_sensor_0_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_1_sensor_0_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_1_sensor_1_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_1_sensor_1_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_1_sensor_1_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_1_sensor_2_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_1_sensor_2_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_1_sensor_2_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)    \
    DATA(pressure_driver_1_sensor_3_voltage_zero, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)  \
    DATA(pressure_driver_1_sensor_3_voltage_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)      \
    DATA(pressure_driver_1_sensor_3_pressure_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)
// jeżeli ktokolwiek usunie tą linie to kompilator zacznie drzeć ryja (chyba że dodasz pustą linię po ostatniej definicji :)