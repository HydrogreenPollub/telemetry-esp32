#include "header.h"
#include "mqtt_control.h"
#include "sdcard_control.h"
#include "uart_control.h"
#include "wifi_control.h"
#include "CANopenNode_ESP32.h"
#include "OD.h"
#include "gps_component.h"
#include "proto_control.h"

telemetry_server_data_t ts_data;
master_telemetry_data_t mt_data;

params_send_mqtt_t ts_params = {
    .buffer_len = 0,
    .serialized_data = {},
};

params_send_mqtt_t mt_params = {
    .buffer_len = 0,
    .serialized_data = {},
};

#define TIME_ZONE (+2)
#define YEAR_BASE (2000)
static const char* TAG = "gps_demo";

static void gps_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    gps_t* gps = NULL;
    switch (event_id)
    {
        case GPS_UPDATE:
            gps = (gps_t*) event_data;
            /* print information parsed from GPS statements */
            ESP_LOGI(TAG,
                "%d/%d/%d %d:%d:%d => \r\n"
                "\tlatitude   = %.05f°N\r\n"
                "\tlongitude = %.05f°E\r\n"
                "\taltitude   = %.02fm\r\n"
                "\tspeed      = %fm/s",
                gps->date.year + YEAR_BASE, gps->date.month, gps->date.day, gps->tim.hour + TIME_ZONE, gps->tim.minute,
                gps->tim.second, gps->latitude, gps->longitude, gps->altitude, gps->speed);
            break;
        case GPS_UNKNOWN:
            /* print unknown statements */
            ESP_LOGW(TAG, "Unknown statement:%s", (char*) event_data);
            break;
        default:
            break;
    }
}

static void start_gps()
{
    /* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    /* init NMEA parser library */
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
}

void send_telemetry_server_data()
{
    serialize_telemetry_server_data(ts_data, ts_params.serialized_data, &(ts_params.buffer_len));
    xTaskCreatePinnedToCore(mqtt_send_data, "MQTT_DATA_TRANSMITION", 1024 * 4, (void*) &ts_params, 10, &handle_mqtt, 1);
}

void on_uart_receive(uint8_t* rx_buffer, uint32_t size)
{
    memcpy(mt_params.serialized_data, rx_buffer, size);
    mt_params.buffer_len = (ssize_t) size;
    int result = deserialize_master_telemetry_data(&mt_data, mt_params.serialized_data, &(mt_params.buffer_len));

    ts_data.vehicleSpeed = mt_data.vehicleSpeed;
    // TODO add the rest
}

void app_main(void)
{
    // CO_ESP32_init();
    // start_gps();

    wifi_init();
    mqtt_init();
    uart_init(on_uart_receive);

    uint8_t temp[3] = { 'x', 'D', '\0' };

    while (1)
    {
        send_telemetry_server_data();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        uart_send_data(temp, 2);
    }
}