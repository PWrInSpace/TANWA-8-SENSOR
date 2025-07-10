idf_component_register(SRCS "led_driver.c" "tmp1075.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")