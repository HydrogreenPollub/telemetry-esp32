#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include "header.h"
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t handle_mqtt_client;
extern TaskHandle_t handle_mqtt;

void mqtt_logs(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
void mqtt_init(void);
void mqtt_start();
void mqtt_send_data(void* pvParameter);

#endif // !MQTT_CONTROL_H
