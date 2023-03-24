/*
TODO list
1. Connect to wifi +
2. Connect to SQL > OR < Create web soccet? -
3. Create JSON - 
4. Write JSON to SDCARD - 
5. Communication with MASTER - 
6.
*/



#include <stdint.h>
#include <stdio.h>
#include "esp_event.h"
#include "esp_netif_types.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
static const char *TAG = "WiFi";
static const char *TAG1 = "MQTT";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG,"WIFI connecting....");
      break;
    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG,"WIFI connected....");
      break;
    case IP_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG,"IP acq");
      break;
  }
}

void wifi_connection(){
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initiation);
  esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handler,NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,wifi_event_handler, NULL);
  
  wifi_config_t wifi_configuration = {
    .sta ={
      .ssid = "NiePodejrzaneWIFI",
      .password = "zaq1@WSX"
    }};
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
  esp_wifi_start();
  esp_wifi_connect();
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG1, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG1, "MQTT_EVENT_CONNECTED");
      msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
      ESP_LOGI(TAG1, "sent publish successful, msg_id=%d", msg_id);

      msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
      ESP_LOGI(TAG1, "sent subscribe successful, msg_id=%d", msg_id);

      msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
      ESP_LOGI(TAG1, "sent subscribe successful, msg_id=%d", msg_id);

      msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
      ESP_LOGI(TAG1, "sent unsubscribe successful, msg_id=%d", msg_id);
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG1, "MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG1, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
      ESP_LOGI(TAG1, "sent publish successful, msg_id=%d", msg_id);
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG1, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG1, "MQTT_EVENT_ERROR");
      if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
        ESP_LOGI(TAG1, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
      break;

    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "URL-dopisac-",
    };
 esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

/*
 * int esp_mqtt_client_publish(esp_mqtt_client_handle_t client, const char *topic, const char *data, int len, int qos, int retain
 *int esp_mqtt_client_publish(client,"TOPIC DO USTALENIA",DANE_Z_MASTERA,0,1,0);
 
 */
void app_main(void)
{
  wifi_connection();
  mqtt_app_start();
  vTaskDelay(5000/portTICK_PERIOD_MS);
}
