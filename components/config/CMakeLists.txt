idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES console driver config can hardware cli nvs sd_card mcu_config Board_Data)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")

