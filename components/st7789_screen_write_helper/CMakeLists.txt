idf_component_register(
    SRCS "st7789_screen_write_helper.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_mm st7789_screen_driver font_module
)