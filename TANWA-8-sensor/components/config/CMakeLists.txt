idf_component_register(  SRC_DIRS "."
                        REQUIRES console
                        REQUIRES driver
                        REQUIRES config can
                         INCLUDE_DIRS "."
)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format" "-Wall" "-Werror")