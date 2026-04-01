#include "pressure_driver.h"

// DATA(name, type, default_value)
// DATA_ARRAY(name, type, array_size, default_value)
// SECTION_BEGIN(name)
// SECTION_END(name)

// obecnie wspierane typy: int32_t, uint8_t, float, double, char, char[]
// możliwość rozszerzenia wspieranych typów w pliku flash.c (należy na samym dole dodać parser i zaktualizować funkcje update_field)

#define CONFIG_FIELDS                                                           \
    SECTION_BEGIN(press_calibr)                                                 \
    DATA(driver_0_0_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_0_0_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_0_0_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_0_1_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_0_1_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_0_1_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_0_2_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_0_2_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_0_2_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_0_3_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_0_3_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_0_3_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_1_0_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_1_0_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_1_0_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_1_1_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_1_1_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_1_1_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_1_2_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_1_2_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_1_2_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    DATA(driver_1_3_volt_0, float, PRESSURE_DRIVER_DEFAULT_ZERO_VOLTAGE)        \
    DATA(driver_1_3_volt_1, float, PRESSURE_DRIVER_DEFAULT_MAX_VOLTAGE)         \
    DATA(driver_1_3_press_1, float, PRESSURE_DRIVER_DEFAULT_MAX_PRESSURE)       \
    SECTION_END(press_calibr)
// jeżeli ktokolwiek usunie tą linie to kompilator zacznie drzeć ryja, chyba że dodasz pustą linię po ostatniej definicji :)