idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES console driver config can hardware cli mcu_config Board_Data nvs_flash)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")

