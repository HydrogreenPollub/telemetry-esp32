#include "wifi_control.h"
const char *TAG_WIFI = "WiFi";
bool wifi_connected=0;

/////////////WIFI section
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG_WIFI,"WIFI connecting....");
      break;
    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG_WIFI,"WIFI connected....");
      break;
    case IP_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG_WIFI,"IP acq");
      wifi_connected = 1;
      gpio_set_level(CONFIG_STS_LED, 255);
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGE(TAG_WIFI,"lost connection");
      wifi_connected = 0;
      gpio_set_level(CONFIG_STS_LED, 0);
      esp_wifi_connect();
      break;
  }
}

void wifi_init(){
  nvs_flash_init(); //non volatile memory initialization
  esp_netif_init();//
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initiation);
  esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event_handler,NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,wifi_event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP,wifi_event_handler, NULL);
  
  wifi_config_t wifi_configuration = {
    .sta ={
      .ssid = CONFIG_ESP_WIFI_SSID,
      .password = CONFIG_ESP_WIFI_PASSWORD
    }};
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

  wifi_start();
}

void wifi_start(){
  esp_wifi_start();
  esp_wifi_connect();
    if(wifi_has_connected()==false) 
  {
    ESP_LOGE(TAG_WIFI,"ERROR connecting to WIFI, RESTART!!!");
    esp_restart();
  }
  ESP_LOGI(TAG_WIFI,"Connected to WIFI");
  
}

int wifi_has_connected(){
  int rep = 0;
  while(!wifi_connected){
    ESP_LOGI(TAG_WIFI,"Connecting to WIFI");
    vTaskDelay(500/portTICK_PERIOD_MS);
    rep++;
    //if(rep == 10 ) return 0;
  }
  return 1;
}