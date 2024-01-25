#include "header.h"
#include "mqtt_control.h"
#include "uart_control.h"
#include "sdcard_control.h"
#include "wifi_control.h"




const char *TAG_WIFI = "WiFi";

struct dataS data;

/////////////main section
void app_main(void)
{    
  gpio_reset_pin(CONFIG_STS_LED);
  gpio_set_direction(CONFIG_STS_LED, GPIO_MODE_OUTPUT);
  // POTRZEBNE?
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("outbox", ESP_LOG_VERBOSE);
  // ?????????????  
  int rep = 0;

  wifi_init();
  wifi_start();

  while(!wifi_ready){
    ESP_LOGI(TAG_WIFI,"Connecting to WIFI");
    vTaskDelay(500/portTICK_PERIOD_MS);
    rep++;
    if(rep == 5 ){ ESP_LOGI(TAG_WIFI,"SUICIDE"); esp_restart();}
  }
  gpio_set_level(CONFIG_STS_LED, 255);
  
  mqtt_init();
  mqtt_start();
  vTaskDelay(1000/portTICK_PERIOD_MS);
  
  //sd_card_init();
  xTaskCreatePinnedToCore(uart_init, "uart_echo_task",1024*4, NULL, 2,&handle_uart,0);
}
