idf_component_register(
    SRCS
        "src/um_seriesd.c"
        "src/um_ws2812_rmt.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c
    PRIV_REQUIRES esp_driver_gpio esp_driver_rmt esp_adc esp_rom
)
