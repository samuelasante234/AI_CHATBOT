idf_component_register(
    SRCS "wifi_setup.c"
    INCLUDE_DIRS "."
    REQUIRES esp_netif esp_wifi nvs_flash esp_event
)