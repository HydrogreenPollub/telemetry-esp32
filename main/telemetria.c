#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif_types.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
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
#include "driver/uart.h"
#include "endian.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

TaskHandle_t mqtt_handle = NULL;
TaskHandle_t handle_SD_Card = NULL;
TaskHandle_t uart = NULL;
// SemaphoreHandle_t semaphore = NULL;  ///do wywalenia
static const char *TAG = "WiFi";
static const char *TAG1 = "MQTT";
static const char *TAG2 = "UART";
static const char *TAG3 = "SD_CARD";
bool wifi_ready=0;
esp_mqtt_client_handle_t client;

struct data {  
  union {
    struct { // TODO remove bitfield since it won't be used here anyway
      uint8_t is_emergency:1;
      uint8_t is_hydrogen_leaking:1;
      uint8_t is_sc_relay_closed:1;
      uint8_t vehicle_is_speed_button_pressed:1;
      uint8_t vehicle_is_half_speed_button_pressed:1;
      uint8_t hydrogen_cell_button_state:2; // Split into two variables?
      uint8_t is_super_capacitor_button_pressed:1;
    };
    uint8_t logic_state; // 4b
  };
  float fc_current; // 8b
  float fc_sc_current; // 12b
  float sc_motor_current; // 16b
  float fc_voltage; // 20b
  float sc_voltage; // 24b
  float hydrogen_sensor_voltage; // 28b
  float fuel_cell_temperature; // 32b
  uint16_t fan_rpm; // 36b
  float vehicle_speed; // 40b
  uint8_t motor_pwm; // 44b
  float hydrogen_pressure; //48b
} data;

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
      .ssid = CONFIG_ESP_WIFI_SSID,
      .password = CONFIG_ESP_WIFI_PASSWORD
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
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG1, "MQTT_EVENT_CONNECTED");
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG1, "MQTT_EVENT_DISCONNECTED");
      break;
      
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG1, "MQTT_SENT");
    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG1, "MQTT_EVENT ERROR CODE, error_code=%d", event->error_handle->error_type);
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
        .broker.address.hostname =  CONFIG_MQTT_HOSTNAME,
        .broker.address.transport = MQTT_TRANSPORT_OVER_TCP,
        .broker.address.port = CONFIG_MQTT_PORT,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWD
        
    };
   client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
void send_data(void *pvParameter){
  //char buff[sizeof(data)];
  char buff[42];
  int i;
  memcpy(&buff,&data,42);
  for (i = 0; i < sizeof(data); i++)
  {
    // if (i > 0) printf(":");
    // printf("%02X ", buff[i]);
  }
  //printf("\n");
  printf("Frame length: %d \n", sizeof(data));
  printf("Buffer length: %d \n", sizeof(buff));
  ESP_LOG_BUFFER_HEXDUMP(TAG2, buff, sizeof(data), 3);
  esp_mqtt_client_publish(client,"/sensors",buff,sizeof(data),1,0);
  vTaskDelete(mqtt_handle);
}

static esp_err_t sd_card_start(){
  esp_err_t ret;
     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0
    };
    sdmmc_card_t *card;
    const char mount_point[] = CONFIG_MOUNT_POINT;
    ESP_LOGI(TAG3, "Initializing SD card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_PIN_NUM_MOSI,
        .miso_io_num = CONFIG_PIN_NUM_MISO,
        .sclk_io_num = CONFIG_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
  return ret;
}

static void sd_card_write(void *arg){
  const char *file_path = CONFIG_MOUNT_POINT"/SD_data.txt";
  ESP_LOGI(TAG, "Opening file %s", CONFIG_MOUNT_POINT);
  FILE *f = fopen(file_path, "a");
  if (f == NULL) {
      ESP_LOGE(TAG, "Failed to open file for writing");
      return;
  }
  fprintf(f,"%hhu,%f,%f,%f,%f,%f,%f,%f,%hu,%f,%hhu,%f\n",data.logic_state,data.fc_current,data.fc_sc_current,data.sc_motor_current,
          data.fc_voltage,data.sc_voltage,data.hydrogen_sensor_voltage,data.fuel_cell_temperature,
          data.fan_rpm,data.vehicle_speed,data.motor_pwm,data.hydrogen_pressure);

  fclose(f);
  ESP_LOGI(TAG, "File written");
  vTaskDelete(handle_SD_Card);
}

static void UART_TASK(void *arg){

  const uart_port_t uart_num = UART_NUM_2;
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_LOGI(TAG2, "Start UART & RS485 application configuration.");
  ESP_ERROR_CHECK(uart_driver_install(uart_num, CONFIG_BUF_SIZE * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, CONFIG_TXD_PIN, CONFIG_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
  ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, 3));

  uint8_t *rx_data = (uint8_t *) malloc(CONFIG_BUF_SIZE);
  while(1){

    int len = uart_read_bytes(uart_num, rx_data, CONFIG_BUF_SIZE, 100 / portTICK_PERIOD_MS);

    if (len > 0) {

      memcpy(&data, rx_data, sizeof(data));
      printf("Frame length: %d \n", sizeof(data));
      printf("Buffer length: %d \n", sizeof(rx_data));
      
      ESP_LOGI(TAG2, "%hhu,%f,%f,%f,%f,%f,%f,%f,%hu,%f,%hhu,%f\n", data.logic_state, data.fc_current, data.fc_sc_current, data.sc_motor_current,
          data.fc_voltage, data.sc_voltage, data.hydrogen_sensor_voltage, data.fuel_cell_temperature,
          data.fan_rpm, data.vehicle_speed, data.motor_pwm, data.hydrogen_pressure);
      
      ESP_LOG_BUFFER_HEXDUMP(TAG2, rx_data, sizeof(rx_data), 2);

      xTaskCreatePinnedToCore(send_data, "MQTT_DATA_TRANSMITION",1024*2, NULL, 10, &mqtt_handle,1);
      //xTaskCreatePinnedToCore(sd_card_write,"SD_CARD_WRITE",1024*2,NULL,10,&handle_SD_Card,1);
    }
  }
}

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
  wifi_connection();
  while(!wifi_ready){
    ESP_LOGI(TAG,"Connecting to WIFI");
    vTaskDelay(500/portTICK_PERIOD_MS);
    rep++;
    if(rep == 5 ){ ESP_LOGI(TAG,"SUICIDE"); esp_restart();}
  }
 gpio_set_level(CONFIG_STS_LED, 255);
  mqtt_app_start();
  vTaskDelay(1000/portTICK_PERIOD_MS);
  //sd_card_start();
  xTaskCreatePinnedToCore(UART_TASK, "uart_echo_task",1024*4, NULL, 2,&uart,0);
}
