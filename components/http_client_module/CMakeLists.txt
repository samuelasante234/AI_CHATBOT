idf_component_register(
    SRCS "http_client_module.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_http_client json_module
)