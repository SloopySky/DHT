# DHT is a git submodule

# DHT/src/dht.c and DHT/include added to this component
# can't get dht_handle_data_line_edge placed in iram when using add_subdirectory(DHT)
# due to symbol granularity placement limiations

idf_component_register(SRCS "dht_hal.c" "dht_esp.c" "DHT/src/dht.c"
                       INCLUDE_DIRS "include" "DHT/include"
                       REQUIRES esp_timer esp_driver_gpio
                       LDFRAGMENTS "dht.lf")

# add_subdirectory(DHT)
# target_link_libraries(${COMPONENT_LIB} PUBLIC DHT)

