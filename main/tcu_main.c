#include "header.h"
#include "mqtt_control.h"
#include "sdcard_control.h"
#include "uart_control.h"
#include "wifi_control.h"
#include "CANopenNode_ESP32.h"
#include "OD.h"
#include "gps_component.h"
#include "proto_control.h"

struct VehicleData vehicle_state_data = {
    .isEmergency = 1,
    .isHydrogenLeaking = 0,
    .isScRelayClosed = 0,
    .vehicleIsSpeedButtonPressed = 0,
    .vehicleIsHalfSpeedButtonPressed = 1,
    .hydrogenCellOneButtonState = 1,
    .hydrogenCellTwoButtonState = 1,
    .isSuperCapacitorButtonPressed = 1,
    .logicState = 12,
    .fcCurrent = 2.1,
    .fcScCurrent = 2.2,
    .scMotorCurrent = 2.3,
    .fcVoltage = 2.4,
    .scVoltage = 2.5,
    .hydrogenSensorVoltage = 2.6,
    .fuelCellTemperature = 2.7,
    .fanRpm = 5,
    .vehicleSpeed = 2.8,
    .motorPwm = 12,
    .hydrogenPressure = 2.9,
};

params_send_mqtt_t send_params = {
    .buffer_len = 0,
    .serialized_vehicle_data = {},
};

#define TIME_ZONE (+8)   // Beijing Time
#define YEAR_BASE (2000) // date in GPS starts from 2000

static const char* TAG = "gps_demo";

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
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

void test_data_send()
{
    serialize_vehicle_data(&vehicle_state_data, &(send_params.serialized_vehicle_data), &(send_params.buffer_len));
    xTaskCreatePinnedToCore(mqtt_send_data, "MQTT_DATA_TRANSMITION", 1024 * 2, &send_params, 10, &handle_mqtt, 1);
}

void app_main(void)
{
    // gpio_reset_pin(CONFIG_STS_LED);
    // gpio_set_direction(CONFIG_STS_LED, GPIO_MODE_OUTPUT);

    // CO_ESP32_init();
    // start_gps();
    wifi_init();
    mqtt_init();
    while (1)
    {
        test_data_send();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // xTaskCreatePinnedToCore(uart_init, "uart_echo_task", 1024 * 4, NULL, 2, &handle_uart, 0);
}