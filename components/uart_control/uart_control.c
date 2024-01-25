
#include "uart_control.h"
#include "mqtt_control.h"

TaskHandle_t handle_uart = NULL;
const char *TAG_UART = "UART";
///////////// UART section

void uart_start(){
  uint8_t *rx_data = (uint8_t *) malloc(CONFIG_BUF_SIZE);
  while(1){

    int len = uart_read_bytes(CONFIG_UART_NUM, rx_data, CONFIG_BUF_SIZE, 100 / portTICK_PERIOD_MS);

    if (len > 0) {

      memcpy(&data, rx_data, sizeof(data));
      printf("Frame length: %d \n", sizeof(data));
      printf("Buffer length: %d \n", sizeof(rx_data));
      
      ESP_LOGI(TAG_UART, "%hhu,%f,%f,%f,%f,%f,%f,%f,%hu,%f,%hhu,%f\n", data.logic_state, data.fc_current, data.fc_sc_current, data.sc_motor_current,
          data.fc_voltage, data.sc_voltage, data.hydrogen_sensor_voltage, data.fuel_cell_temperature,
          data.fan_rpm, data.vehicle_speed, data.motor_pwm, data.hydrogen_pressure);
      
      ESP_LOG_BUFFER_HEXDUMP(TAG_UART, rx_data, sizeof(rx_data), 2);

      xTaskCreatePinnedToCore(mqtt_send_data, "MQTT_DATA_TRANSMITION",1024*2, NULL, 10, &handle_mqtt,1);
      //xTaskCreatePinnedToCore(sd_card_write,"SD_CARD_WRITE",1024*2,NULL,10,&handle_SD_Card,1); 
    }
  }
}


void uart_init(void *arg){

  const uart_port_t uart_num = UART_NUM_2;
  uart_config_t uart_config = {
    .baud_rate = CONFIG_UART_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_LOGI(TAG_UART, "Start UART & RS485 application configuration.");
  ESP_ERROR_CHECK(uart_driver_install(uart_num, CONFIG_BUF_SIZE * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, CONFIG_TXD_PIN, CONFIG_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
  ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, 3));


  uart_start();

}
