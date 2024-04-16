#include "header.h"
#include "mqtt_control.h"
#include "sdcard_control.h"
#include "uart_control.h"
#include "wifi_control.h"

vehicle_state_frame_t vehicle_state_data;

void send_data()
{
    for (int i = 0; i < 10; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        xTaskCreatePinnedToCore(mqtt_send_data, "MQTT_DATA_TRANSMITION", 1024 * 2, NULL, 10, &handle_mqtt, 1);
    }
}

void app_main(void)
{
    gpio_reset_pin(CONFIG_STS_LED);
    gpio_set_direction(CONFIG_STS_LED, GPIO_MODE_OUTPUT);

    wifi_init();
    mqtt_init();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(uart_init, "uart_echo_task", 1024 * 4, NULL, 2, &handle_uart, 0);
}