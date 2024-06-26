#include "mqtt_control.h"

const char* TAG_MQTT = "MQTT";

esp_mqtt_client_handle_t handle_mqtt_client;
TaskHandle_t handle_mqtt = NULL;

void mqtt_logs(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    handle_mqtt_client = event->client;
    switch ((esp_mqtt_event_id_t) event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_SENT");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT ERROR CODE, error_code=%d", event->error_handle->error_type);
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                if (event->error_handle->esp_tls_last_esp_err != 0)
                {
                    ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", "reported from esp-tls",
                        event->error_handle->esp_tls_last_esp_err);
                }
                if (event->error_handle->esp_tls_stack_err != 0)
                {
                    ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", "reported from tls stack",
                        event->error_handle->esp_tls_stack_err);
                }
                if (event->error_handle->esp_transport_sock_errno != 0)
                {
                    ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", "captured as transport's socket errno",
                        event->error_handle->esp_transport_sock_errno);
                }
                ESP_LOGI(TAG_MQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG_WIFI, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_init(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.hostname = CONFIG_MQTT_HOSTNAME,
        .broker.address.transport = MQTT_TRANSPORT_OVER_TCP,
        .broker.address.port = CONFIG_MQTT_PORT,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWD,
    };
    handle_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    mqtt_start();
}

void mqtt_start()
{
    esp_mqtt_client_register_event(handle_mqtt_client, ESP_EVENT_ANY_ID, mqtt_logs, NULL);
    esp_mqtt_client_start(handle_mqtt_client);
}

void mqtt_send_data(void* pvParameter)
{
    params_send_mqtt_t* params = pvParameter;
    char buff[params->buffer_len];
    memcpy(&buff, params->serialized_data, params->buffer_len);
    // ESP_LOGI(TAG_MQTT, "Frame length: %d \n", params->buffer_len);
    // ESP_LOGI(TAG_MQTT, "Buffer length: %d \n", sizeof(buff));
    // ESP_LOG_BUFFER_HEXDUMP(TAG_MQTT, buff, params->buffer_len, 3);
    int temp = esp_mqtt_client_publish(handle_mqtt_client, "sensors", buff, params->buffer_len, 1, 0);
    // ESP_LOGE(TAG_MQTT, "mqtt returned val %d", temp);
    vTaskDelete(handle_mqtt);
}