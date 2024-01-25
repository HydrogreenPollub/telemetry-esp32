idf_component_register(SRCS "mqtt_control.c"
                    INCLUDE_DIRS "include"
                    REQUIRES header)
set(COMPONENT_REQUIRES mqtt)