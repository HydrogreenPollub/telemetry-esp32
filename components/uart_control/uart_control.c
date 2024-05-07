#include "uart_control.h"
#include "esp_log.h"

const char* TAG_UART = "UART";
void (*callback)(uint8_t*, uint32_t) = NULL;

void uart_read_task()
{
    uint8_t* rx_data = (uint8_t*) malloc(CONFIG_BUF_SIZE);
    ESP_LOGI(TAG_UART, "RX buffer length after initialization: %d \n", sizeof(rx_data));
    while (1)
    {
        uint32_t len = uart_read_bytes(CONFIG_UART_NUM, rx_data, 64, 20 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            ESP_LOGI(TAG_UART, "Frame length: %d \n", (int) len);
            // ESP_LOGI(TAG_UART, "%hhu,%f,%f,%f,%f,%f,%f,%f,%hu,%f,%hhu,%f\n", vehicle_state_data.logic_state,
            //     vehicle_state_data.fc_current, vehicle_state_data.fc_sc_current, vehicle_state_data.sc_motor_current,
            //     vehicle_state_data.fc_voltage, vehicle_state_data.sc_voltage,
            //     vehicle_state_data.hydrogen_sensor_voltage, vehicle_state_data.fuel_cell_temperature,
            //     vehicle_state_data.fan_rpm, vehicle_state_data.vehicle_speed, vehicle_state_data.motor_pwm,
            //     vehicle_state_data.hydrogen_pressure);
            ESP_LOG_BUFFER_HEXDUMP(TAG_UART, rx_data, len, 2);
            callback(rx_data, len);
        }
    }
}

void uart_init(void (*on_read_callback)(uint8_t*, uint32_t))
{
    callback = on_read_callback;

    const uart_port_t uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_LOGI(TAG_UART, "Start UART & RS422 application configuration.");
    ESP_ERROR_CHECK(uart_driver_install(uart_num, CONFIG_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, CONFIG_TXD_PIN, CONFIG_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
    ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, 3));

    xTaskCreate(uart_read_task, "uart_task", 4 * 1024, NULL, 8, NULL);
}

void uart_send_data(uint8_t* data, uint32_t size)
{
    int txBytes = uart_write_bytes(UART_NUM_2, data, size);
    ESP_LOGI(TAG_UART, "Sent %d bytes", txBytes);
}