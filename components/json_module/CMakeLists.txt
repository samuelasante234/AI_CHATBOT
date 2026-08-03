idf_component_register(
    SRCS "json_module.c"
    INCLUDE_DIRS "."
    REQUIRES driver json http_client_module
)