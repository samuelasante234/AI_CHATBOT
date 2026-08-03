#include "wifi_setup.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif_types.h"
#include "esp_event.h"
#include <stdio.h>

void *wlan_initialise();
void set_station_param();
void register_wlan_event_loops();
static void event_handler_for_wifi(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data);
static void event_handler_for_ip(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data);


void *wlan_initialise() {
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK) {
        printf("Couldn't initialise NVS flash! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    result= esp_netif_init();
    if (result != ESP_OK) {
        printf("Couldn't initialise underlying TCP/IP stack! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    result=esp_event_loop_create_default();
    if (result != ESP_OK) {
        printf("Couldn't create event loop! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return NULL;
    }
    esp_netif_t* netif_pointer;
    netif_pointer=esp_netif_create_default_wifi_sta();
    if (!netif_pointer) {
        printf("Couldn't create netif object! Pointer invalid!\n");
        fflush(stdout);
        return NULL;
    }
    return (void *)netif_pointer;
}
void set_station_param() {
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t result = esp_wifi_init(&init_cfg);
    if (result != ESP_OK) {
        printf("Couldn't initialise Wi-Fi driver! Error: %s\n", esp_err_to_name(result));
        return;
    }
    wifi_config_t sta_config={0};
    result=esp_wifi_set_config(WIFI_IF_STA,&sta_config);
    if (result != ESP_OK) {
        printf("Couldn't configure station parameters! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_wifi_set_mode(WIFI_MODE_STA);
    if (result != ESP_OK) {
        printf("Couldn't set device as WIFI station! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_wifi_start();
    if (result != ESP_OK) {
        printf("Couldn't start WIFI! Error: %s\n", esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
void register_wlan_event_loops() {
    esp_err_t result=esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,event_handler_for_wifi,NULL,NULL);
    if (result != ESP_OK) {
        printf("Couldn't register WIFI event handlers! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
    result=esp_event_handler_instance_register(IP_EVENT,ESP_EVENT_ANY_ID,event_handler_for_ip,NULL,NULL);
    if (result != ESP_OK) {
        printf("Couldn't register IP event handlers! Error: %s\n",esp_err_to_name(result));
        fflush(stdout);
        return;
    }
}
static void event_handler_for_wifi(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data) {
    esp_err_t result;
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                result=esp_wifi_connect();
                if (result != ESP_OK) {
                    printf("Couldn't connect to WIFI! Error: %s\n", esp_err_to_name(result));
                    fflush(stdout);
                    return;
                }
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                result=esp_wifi_connect();
                if (result != ESP_OK) {
                    printf("Couldn't connect to WIFI! Error: %s\n", esp_err_to_name(result));
                    fflush(stdout);
                    return;
                }
                break;
            default:
                return;
        }
    }
}
static void event_handler_for_ip(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data) {
    if (event_base==IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                ip_event_got_ip_t* ip_event = (ip_event_got_ip_t *)(event_data);
                printf("Successfully acquired IP Address: " IPSTR "\n", IP2STR(&ip_event->ip_info.ip));
                fflush(stdout);
                break;
            default:
            return;
        }
    }
}