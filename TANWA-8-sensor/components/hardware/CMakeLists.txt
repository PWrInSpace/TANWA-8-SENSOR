idf_component_register(SRCS "max31856.c" "pressure_driver.c" "tmp1075.c" "led_driver.c" "ads1115.c" "hdc1080.c" "mcp_driver.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver mcu_config "esp-idf-lib__mcp342x" "esp-idf-lib__i2cdev")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")