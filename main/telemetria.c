/*
TODO list
1. Connect to wifi +
2. Connect to MQTT service +
3. Create JSON - 
4. Write JSON to SDCARD - 
5. Communication with MASTER - 
6.
*/
#include <stdint.h>
#include <stdio.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif_types.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

static const char *TAG = "WiFi";
static const char *TAG1 = "MQTT";
static const char *TAG2 = "UART";
static const char *TAG3 = "SD_CARD";
bool wifi_ready=0;
esp_mqtt_client_handle_t client;

struct data{
  float fuel_cell_voltage; //current_fuel_voltage
  float fuel_cell_current;
  float supper_capacitor_current;//supper_capacitor_current
  float supper_capacitor_voltage;
  float speed;
  float fan_RPM;
  float cell_temperature;
  float pressure_hg;
  float motor_current;
  uint8_t error_reason;
  //one bits
  union{
    struct
     {
      uint8_t Speed_button:1;
      uint8_t Half_Speed_button:1;
      uint8_t Emergency:1;
      uint8_t HG_Cell_button:2;
      uint8_t supper_capacitor_button:1;
      uint8_t relay_state:1;
      uint8_t hg_sens:1;
    };
    uint8_t logic_state; //nazwa robocza
  };
 }data;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG1, "Last error %s: 0x%x", message, error_code);
    }
}

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
      wifi_ready = 1;
      break;
  }
}

void wifi_connection(){
  nvs_flash_init(); //non volatile memory initialization
  esp_netif_init();//
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
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG1, "MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG1, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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
    case MQTT_EVENT_DATA:
       ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);  
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
      
  }
}

static void mqtt_app_start(void) 
{
  esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.163.223:1883",
    };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
void send_data(void *pvParameter){
  char buff[4];

  memcpy(&buff,&data.fuel_cell_current,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/fuel_cell_current",buff,0,1,0);

  memcpy(&buff,&data.fuel_cell_voltage,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/fuel_cell_voltage",buff,0,1,0);

  memcpy(&buff,&data.supper_capacitor_current,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/supper_capacitor_current",buff,0,1,0);

  memcpy(&buff,&data.supper_capacitor_voltage,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/supper_capacitor_voltage",buff,0,1,0);

  memcpy(&buff,&data.speed,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/speed",buff,0,1,0);

  memcpy(&buff,&data.fan_RPM,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/fan_RPM",buff,0,1,0);

  memcpy(&buff,&data.cell_temperature,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/cell_temperature",buff,0,1,0);

  memcpy(&buff,&data.pressure_hg,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/pressure_hg",buff,0,1,0);

  memcpy(&buff,&data.motor_current,sizeof(float));
  esp_mqtt_client_publish(client,"/sensory/motor_current",buff,0,1,0);
  char buff2; 
  memcpy(&buff2,&data.logic_state,sizeof(char));
  esp_mqtt_client_publish(client,"/logic",buff,0,1,0);
}

static esp_err_t sd_card_start(){
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
    };
    sdmmc_card_t *card;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK)
    {
        return err;
    }
    return ESP_OK;
}
static esp_err_t sd_card_write(const char *path, char *data){
  ESP_LOGI(TAG, "Opening file %s", path);
  FILE *f = fopen(path, "w");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return ESP_FAIL;
  }
  fprintf(f, data);
  fclose(f);
  ESP_LOGI(TAG, "File written");

  return ESP_OK;
}
void app_main(void)
{
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("outbox", ESP_LOG_VERBOSE);

  wifi_connection();
  while(!wifi_ready){
    ESP_LOGI(TAG,"Connecting to WIFI");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  mqtt_app_start();

}
