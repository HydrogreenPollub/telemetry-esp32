#include "freertos/idf_additions.h"
#include "header.h"
#include "mqtt_control.h"
#include "sdcard_control.h"
#include "driver/i2c_master.h"
#include "uart_control.h"
#include "wifi_control.h"
#include "CANopenNode_ESP32.h"
#include "OD.h"
#include "gps_component.h"
#include "proto_control.h"
#include "mcp342x.h"

#define I2C_PORT 0
#define I2C_MASTER_SCL_IO 19
#define I2C_MASTER_SDA_IO 18
TaskHandle_t handle_adc;



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




void adc_handler()
{
    config_mcp3424_t mcp_conf = {
		._address = 0x6e,
		._resolution = 18,
		._mode = 1,
		._PGA = 0,
		._channel = 2,
	};
	///////////////////////////////////////////////config for i2c
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
///////////////////////////////////////////////////////////////

    uint8_t i = 0;
    while(1)
	{
        for(i = 0; i<4; i++)
        {
            mcp_conf._channel=i;
            mcp342x_init(&mcp_conf, dev_handle);         
            mcp342x_new_conversion(dev_handle, &mcp_conf);
            switch (i) {
                //channel 0 of adc
                case 0:
                    ts_data.mcCurrent = mcp342x_measure(dev_handle,&mcp_conf);
                    break;
                //channel 1 of adc
                case 1:
                    ts_data.hydrogenPressure = mcp342x_measure(dev_handle,&mcp_conf);
                    break;
                //channel 2 of adc
                case 2:
                    ts_data.fcCurrentRaw = mcp342x_measure(dev_handle,&mcp_conf);
                    break;
                //channel 3 of adc
                case 3:
                    ts_data.fcVoltageRaw = mcp342x_measure(dev_handle,&mcp_conf);
                    break;
                default:
                    break;
            }
            ESP_LOGI("I2C - meas", "channel: %d, value: %lf", i, mcp342x_measure(dev_handle,&mcp_conf));
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
	}///////////end of while
}
void app_main(void)
{
    // CO_ESP32_init();
    // start_gps();

    wifi_init();
    mqtt_init();
    uart_init(on_uart_receive);

    uint8_t temp[3] = { 'x', 'D', '\0' };
    
    xTaskCreatePinnedToCore(adc_handler, "adc read", 1024 * 4, NULL, 10, &handle_adc, 1);
    while (1)
    {
        send_telemetry_server_data();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        uart_send_data(temp, 2);
    }
}