#include "freertos/idf_additions.h"
#include "header.h"
#include "mqtt_control.h"
#include "sdcard_control.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "uart_control.h"
#include "wifi_control.h"
#include "CANopenNode_ESP32.h"
#include "OD.h"
#include "gps_component.h"
#include "proto_control.h"
#include "mcp342x.h"

#define I2C_PORT          0
#define I2C_MASTER_SCL_IO 32
#define I2C_MASTER_SDA_IO 21
#define MAP_2V_MAX 32088
TaskHandle_t handle_adc;

telemetry_server_data_t ts_data;
master_telemetry_data_t mt_data;
static bool has_received_uart_data = false;

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
    ESP_LOGI("GPS","handler_enter");
    gps_t* gps = NULL;
    switch (event_id)
    {
        case GPS_UPDATE:
            gps = (gps_t*) event_data;
            /* print information parsed from GPS statements */
            ESP_LOGI(TAG,
                "%d/%d/%d %d:%d:%d => \r\n"
                "\tlatitude   = %.05f°N\r\n"
                "\tlongitude  = %.05f°E\r\n"
                "\taltitude   = %.02fm\r\n"
                "\tspeed      = %fm/s",
                gps->date.year + YEAR_BASE, gps->date.month, gps->date.day, gps->tim.hour + TIME_ZONE, gps->tim.minute,
                gps->tim.second, gps->latitude, gps->longitude, gps->altitude, gps->speed);

            ts_data.gpsAltitude = gps->latitude;
            ts_data.gpsLatitude = gps->latitude;
            ts_data.gpsLongitude = gps->longitude;
            ts_data.gpsSpeed = gps->speed;

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
    if(nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL)==ESP_OK)
        ESP_LOGI("GPS","add hander ok");
    else 
        ESP_LOGI("GPS","add handler fail");
    
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

    ts_data.isEmergency = mt_data.isEmergency;
    ts_data.isEmergencyButtonPressed = mt_data.isEmergencyButtonPressed;
    ts_data.isEmergencySwitchToggled = mt_data.isEmergencySwitchToggled;
    ts_data.isHydrogenLeaking = mt_data.isHydrogenLeaking;
    ts_data.isScRelayClosed = mt_data.isScRelayClosed;
    ts_data.isTimeResetButtonPressed = mt_data.isTimeResetButtonPressed;
    ts_data.isHalfSpeedButtonPressed = mt_data.isHalfSpeedButtonPressed;
    ts_data.isGasButtonPressed = mt_data.isGasButtonPressed;
    ts_data.fuelCellMode = mt_data.fuelCellMode;

    ts_data.fcCurrent = mt_data.fcCurrent;
    ts_data.fcScCurrent = mt_data.fcScCurrent;
    ts_data.scMotorCurrent = mt_data.scMotorCurrent;
    ts_data.fcVoltage = mt_data.fcVoltage;
    ts_data.scVoltage = mt_data.scVoltage;
    ts_data.hydrogenSensorVoltage = mt_data.hydrogenSensorVoltage;
    ts_data.fuelCellTemperature = mt_data.fuelCellTemperature;
    ts_data.fanRpm = mt_data.fanRpm;
    ts_data.vehicleSpeed = mt_data.vehicleSpeed;
    ts_data.motorPwm = mt_data.motorPwm;
    // ts_data.hydrogenPressure = mt_data.hydrogenPressure;

    /*
    ts_data.motorSpeed = mt_data.motorSpeed;
    ts_data.motorCurrent = mt_data.motorCurrent;
    ts_data.fcCurrentRaw = mt_data.fcCurrentRaw;
    ts_data.fcVoltageRaw = mt_data.fcVoltageRaw;
    ts_data.mcCurrent = mt_data.mcCurrent;
    */
    ts_data.lapNumber = mt_data.lapNumber;

    has_received_uart_data = true;
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void adc_handler()
{
    config_mcp3424_t mcp_conf = {
        ._address = 0x6e,
        ._resolution = 18,
        ._mode = 1,
        ._PGA = 0,
        ._channel = 2,
    };

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x6e,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    ESP_LOGI("I2C", "bus add device mcp3424");

    uint8_t i = 0;
    while (1)
    {
        for (i = 0; i < 4; i++)
        {
            // i=1;
            mcp_conf._channel = i;
            mcp342x_init(&mcp_conf, dev_handle);
            mcp342x_new_conversion(dev_handle, &mcp_conf);
            
            switch (i)
            {
                // channel 0 of adc
                case 0:
                    ts_data.mcCurrent = map(mcp342x_measure(dev_handle, &mcp_conf), 0, MAP_2V_MAX, 0, 50);
                    ESP_LOGI("I2C - meas", "channel: %d, value: %f\n\n", i, ts_data.mcCurrent);
                    break;
                // channel 1 of adc
                case 1:
                    ts_data.hydrogenPressure = map(mcp342x_measure(dev_handle, &mcp_conf), 0, MAP_2V_MAX, 0, 50);
                    ESP_LOGI("I2C - meas", "channel: %d, value: %f\n\n", i, ts_data.hydrogenPressure);
                    break;
                // channel 2 of adc
                case 2:
                    ts_data.fcCurrentRaw = map(mcp342x_measure(dev_handle, &mcp_conf), 0, MAP_2V_MAX, 0, 50);
                    ESP_LOGI("I2C - meas", "channel: %d, value: %f\n\n", i, ts_data.fcCurrentRaw);
                    break;
                // channel 3 of adc
                case 3:
                    ts_data.fcVoltageRaw = map(mcp342x_measure(dev_handle, &mcp_conf), 0, MAP_2V_MAX, 0, 50);
                    ESP_LOGI("I2C - meas", "channel: %d, value: %f\n\n", i, ts_data.fcVoltageRaw);
                    break;
                default:
                    break;
            }
            
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

static void status_led_init()
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = 22,
        .duty = 0, // (2^13 - 1) * 50% = 4096
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void app_main(void)
{
    status_led_init();

    start_gps();
    wifi_init();
    mqtt_init();
    uart_init(on_uart_receive);
    // xTaskCreatePinnedToCore(adc_handler, "adc_read", 1024 * 4, NULL, 10, &handle_adc, 1);

    while (1)
    {
        // if (has_received_uart_data)
        // {
            send_telemetry_server_data();
            // ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191));
            // ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
        // }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // uint8_t temp[4] = { 0x12, 'x', 'D', 0x17 };
        // uart_send_data(temp, 4);
    }
}