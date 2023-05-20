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
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define STS_LED (GPIO_NUM_22)
#define BUF_SIZE (1024*4)
#define PIN_NUM_MISO  CONFIG_EXAMPLE_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_EXAMPLE_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_EXAMPLE_PIN_CLK
#define PIN_NUM    CONFIG_EXAMPLE_PIN_CS
#define MOUNT_POINT "/sdcard"
TaskHandle_t mqtthandle = NULL;
TaskHandle_t SDCardHandle = NULL;
TaskHandle_t UART = NULL;
SemaphoreHandle_t Semaphore = NULL;
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
      uint8_t is_SC_relay_closed:1;
      uint8_t vehicle_is_speed_button_pressed:1;
      uint8_t vehicle_is_half_speed_button_pressed:1;
      uint8_t hydrogen_cell_button_state:2; // Split into two variables?
      uint8_t is_super_capacitor_button_pressed:1;
    };
    uint8_t logic_state;
  };
  float FC_current;
  float FC_SC_current;
  float SC_motor_current;
  float FC_voltage;
  float SC_voltage;
  float Hydrogen_sensor_voltage;
  float fuel_cell_temperature;
  uint16_t fan_rpm;
  float vehicle_speed;
  uint8_t motor_PWM;
  float hydrogen_pressure;
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
  char buff[sizeof(data)];
  int i;
  memcpy(&buff,&data,sizeof(data));
  for (i = 0; i < sizeof(data); i++)
  {
    if (i > 0) printf(":");
    printf("%02X ", buff[i]);
  }
printf("\n");
  esp_mqtt_client_publish(client,"/sensors",buff,sizeof(data),1,0);
  vTaskDelete(mqtthandle);
}

static esp_err_t sd_card_start(){
  esp_err_t ret;
     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG3, "Initializing SD card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_PIN_NUM_MOSI,
        .miso_io_num =  CONFIG_PIN_NUM_MISO,
        .sclk_io_num =  CONFIG_PIN_NUM_CLK,
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
  const char *file_path = MOUNT_POINT"/SD_data.txt";
  ESP_LOGI(TAG, "Opening file %s", MOUNT_POINT);
  FILE *f = fopen(file_path, "a");
  if (f == NULL) {
      ESP_LOGE(TAG, "Failed to open file for writing");
      return;
  }
  fprintf(f,"%hhu,%f,%f,%f,%f,%f,%f,%f,%hu,%f,%hhu,%f\n",data.logic_state,data.FC_current,data.FC_SC_current,data.SC_motor_current,
          data.FC_voltage,data.SC_voltage,data.Hydrogen_sensor_voltage,data.fuel_cell_temperature,
          data.fan_rpm,data.vehicle_speed,data.motor_PWM,data.hydrogen_pressure);
/*  
  char data_to_write[5] = {0,0,0,0,44};
  memcpy(data_to_write,&data.logic_state,sizeof(float));
  fprintf(f, &data_to_write);

  memcpy(data_to_write,&data.FC_current,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.FC_SC_current,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.SC_motor_current,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.FC_voltage,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.SC_voltage,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.Hydrogen_sensor_voltage,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.fuel_cell_temperature,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.fan_rpm,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.vehicle_speed,sizeof(float));
  fprintf(f, &data_to_write);
  
  memcpy(data_to_write,&data.motor_PWM,sizeof(float));
  fprintf(f, &data_to_write);

  memcpy(data_to_write,&data.hydrogen_pressure,sizeof(float));
  fprintf(f, &data_to_write);
  fprintf(f,"\n");
  */
  fclose(f);
  ESP_LOGI(TAG, "File written");
  vTaskDelete(SDCardHandle);
}

static void UART_TASK(void *arg){
  // Configure UART parameters
  const uart_port_t uart_num = UART_NUM_2;
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .source_clk = UART_SCLK_DEFAULT,
  };
  // Set UART log level
  ESP_LOGI(TAG2, "Start RS485 application configure UART.");
   // Install UART driver
  ESP_ERROR_CHECK(uart_driver_install(uart_num,BUF_SIZE, 0, 0, NULL, 0));
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_LOGI(TAG, "UART set pins, mode and install driver.");
  //configure UART Pins
  ESP_ERROR_CHECK(uart_set_pin(uart_num, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  uint8_t *rx_data = (uint8_t *) malloc(BUF_SIZE);
  while(1){
    
    int len = uart_read_bytes(uart_num, rx_data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
    if(len>0){

      //ESP_LOGI(TAG2,"%d, BUF: %u, DATA: %u",len,sizeof(rx_data),sizeof(data));
      memcpy(&data,rx_data,sizeof(data));
/*      ESP_LOGI(TAG2,"%f",data.fuel_cell_voltage);
      ESP_LOGI(TAG2,"%f",data.fuel_cell_current);
      ESP_LOGI(TAG2,"%f",data.super_capacitor_current);
      ESP_LOGI(TAG2,"%f",data.super_capacitor_voltage);
      ESP_LOGI(TAG2,"%f",data.vehicle_speed);
      ESP_LOGI(TAG2,"%f",data.fan_rpm);
      ESP_LOGI(TAG2,"%f",data.fuel_cell_temperature);
      ESP_LOGI(TAG2,"%f",data.hydrogen_pressure);
      ESP_LOGI(TAG2,"%f",data.motor_current);
      ESP_LOGI(TAG2,"%d",data.error_codes);
      ESP_LOGI(TAG2,"%d",data.logic_state);
  */    xTaskCreatePinnedToCore(send_data, "MQTT_DATA_TRANSMITION",1024*2, NULL, 10, &mqtthandle,1);
        xTaskCreatePinnedToCore(sd_card_write,"SD_CARD_WRITE",1024*2,NULL,10,&SDCardHandle,1);
        }
    }
  }


void app_main(void)
{    
gpio_reset_pin(STS_LED);
  gpio_set_direction(STS_LED, GPIO_MODE_OUTPUT);
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
 gpio_set_level(STS_LED, 255);
  mqtt_app_start();
  vTaskDelay(1000/portTICK_PERIOD_MS);
  sd_card_start();
  xTaskCreatePinnedToCore(UART_TASK, "uart_echo_task",1024*4, NULL, 2,&UART,0);
}
