#ifndef HTTP_CLIENT_MODULE
#define HTTP_CLIENT_MODULE

#include "esp_http_client.h"

esp_http_client_handle_t http_post_config();
esp_err_t http_event_handler(esp_http_client_event_t *evt);
void fire_payload(esp_http_client_handle_t http_client_handle, char* payload);

extern esp_http_client_handle_t http_client_handle;
#endif