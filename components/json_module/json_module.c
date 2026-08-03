#include "json_module.h"
#include "http_client_module.h"
#include "cJSON.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void json_payload_transact();

void json_payload_transact() {
    cJSON *json_head=cJSON_CreateObject();
    if (!json_head) {
        printf("Couldn't allocate JSON list head!");
        fflush(stdout);
        cJSON_Delete(json_head);
        return;
    }
    cJSON *string_item;
    string_item = cJSON_AddStringToObject(json_head,"status_message","Status: Active");
    if (!string_item) {
        printf("Add Item to JSON list fail!");
        fflush(stdout);
        cJSON_Delete(json_head);
        return;
    }
    else {
        printf("Add Item to JSON list success!");
        fflush(stdout);
    }
    char* payload=cJSON_PrintUnformatted(json_head);
    if (!payload) {
        printf("Payload content NULL!");
        fflush(stdout);
        cJSON_Delete(json_head);
        cJSON_free(payload);
        return;
    }
    fire_payload(http_client_handle,payload);
    cJSON_Delete(json_head);
    cJSON_free(payload);
}