#include "proto_control.h"

ssize_t buffer_len = 0;
uint8_t buf[4096];

int serialize_telemetry_server_data(telemetry_server_data_t ts_data, uint8_t* output_data, ssize_t* output_data_len)
{
    // Create capnproto object
    struct capn c;
    capn_init_malloc(&c);

    // Create pointer to root structure
    capn_ptr cr = capn_root(&c);
    struct capn_segment* cs = cr.seg;

    // Create pointer to data
    TSData_ptr ptr = new_TSData(cs);
    write_TSData(&ts_data, ptr);
    int setp_ret = capn_setp(capn_root(&c), 0, ptr.p);
    // ASSERT_EQ(0, setp_ret);

    // Write structure data to buffer
    *output_data_len = capn_write_mem(&c, output_data, 4096, 0 /* packed */);

    // ESP_LOG_BUFFER_HEXDUMP("TAG", output_data, 64, ESP_LOG_WARNING);
    capn_free(&c);

    return setp_ret;
}

int deserialize_master_telemetry_data(master_telemetry_data_t* ms_data, uint8_t* input_data, ssize_t* input_data_len)
{
    ESP_LOG_BUFFER_HEXDUMP("TAG_UART", input_data, *input_data_len, 2);

    // Create capnproto object
    struct capn c;
    int init_mem_ret = capn_init_mem(&c, input_data, *input_data_len, 0 /* packed */);

    // Create pointer to root structure
    MTData_ptr root_p;
    root_p.p = capn_getp(capn_root(&c), 0, 1);
    read_MTData(ms_data, root_p);

    // ESP_LOG_BUFFER_HEXDUMP("TAG", output_data, 64, ESP_LOG_WARNING);
    capn_free(&c);

    return init_mem_ret;
}