#include "uart_control.h"
#include "esp_log.h"

const char* TAG_UART = "UART";
const uint8_t UART_EXPECTED_LENGTH = 80;

void (*callback)(uint8_t*, uint32_t) = NULL;

void uart_read_task()
{
    uint8_t* rx_data = (uint8_t*) malloc(CONFIG_BUF_SIZE);
    ESP_LOGI(TAG_UART, "RX buffer length after initialization: %d \n", sizeof(rx_data));
    while (1)
    {
        uint32_t len = uart_read_bytes(CONFIG_UART_NUM, rx_data, 128, 64 / portTICK_PERIOD_MS);
        if (len == UART_EXPECTED_LENGTH)
        {
            ESP_LOGI(TAG_UART, "Frame length: %d", (int) len);

            // ESP_LOG_BUFFER_HEXDUMP(TAG_UART, rx_data, len, 2);
            callback(rx_data, len);
        }
        else if (len != UART_EXPECTED_LENGTH && len > 0)
        {
            ESP_LOGI(TAG_UART, "Frame with %lu size has been received (expected: %d)", len, 64);
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
    ESP_ERROR_CHECK(uart_driver_install(uart_num, CONFIG_BUF_SIZE * 4, CONFIG_BUF_SIZE * 4, CONFIG_BUF_SIZE, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, CONFIG_TXD_PIN, CONFIG_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, 3));

    xTaskCreate(uart_read_task, "uart_task", 4 * 1024, NULL, 8, NULL);
}

void uart_send_data(uint8_t* data, uint32_t size)
{
    int txBytes = uart_write_bytes(UART_NUM_2, data, size);
    ESP_LOGI(TAG_UART, "Sent %d bytes", txBytes);
}