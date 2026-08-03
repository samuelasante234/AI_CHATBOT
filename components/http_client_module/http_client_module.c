#include "http_client_module.h"
#include "esp_http_client.h"
#include "json_module.h"
#include <string.h>

esp_http_client_handle_t http_post_config();
esp_err_t http_event_handler(esp_http_client_event_t *evt);
void fire_payload(esp_http_client_handle_t http_client_handle, char* payload);

esp_http_client_handle_t http_post_config() {
    esp_http_client_handle_t http_client_handle=NULL;
    esp_http_client_config_t http_client={
        .url="http://10.160.79.10:8000/chat",
        .method=HTTP_METHOD_POST,
        .event_handler=http_event_handler,
    };
    http_client_handle=esp_http_client_init(&http_client);
    if (!http_client_handle) {
        //a permanent solution will be developed to make sure the esp stops here. for now, let's take it like this
        printf("NULL http client handle");
        fflush(stdout); 
    }
    return http_client_handle;
}
esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            printf("TCP handshake successful!");
            fflush(stdout);
            return ESP_OK;
            break;
        case HTTP_EVENT_ON_DATA:
            printf("%.*s",evt->data_len,(char*)evt->data);
            fflush(stdout);
            return ESP_OK;
            break;
        case HTTP_EVENT_ON_FINISH:
            printf("TCP transaction completed!");
            fflush(stdout);
            return ESP_OK;
            break;
        default:
            return ESP_OK;
    }
}
void fire_payload(esp_http_client_handle_t http_client_handle, char* payload) {
    if (!http_client_handle) {
        printf("Cannot proceed to fire payload! Error: NULL handle");
        fflush(stdout);
        return;
    }
    esp_err_t result=esp_http_client_set_header(http_client_handle, "Content-Type", "application/json");
    if (result != ESP_OK) {
        printf("Couldn't establish http header! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_http_client_set_post_field(http_client_handle,payload, strlen(payload));
    if (result != ESP_OK) {
        printf("Couldn't load http payload into buffer to transmit! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_http_client_perform(http_client_handle);
    if (result != ESP_OK) {
        printf("Couldn't perform http transaction! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_http_client_cleanup(http_client_handle);
    if (result != ESP_OK) {
        printf("Couldn't clean left-over http memory! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}