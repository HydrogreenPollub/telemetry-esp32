#include "proto_control.h"

void serialize_vehicle_data(vehicle_data_t* vehicle_state_data, uint8_t* serialized_vehicle_data, ssize_t* buffer_len)
{
    struct capn c;
    capn_init_malloc(&c);
    capn_ptr cr = capn_root(&c);
    struct capn_segment* cs = cr.seg;
    VehicleData_ptr ptr = new_VehicleData(cs);
    write_VehicleData(&vehicle_state_data, ptr);
    int setp_ret = capn_setp(capn_root(&c), 0, ptr.p);
    // ASSERT_EQ(0, setp_ret);
    *buffer_len = capn_write_mem(&c, serialized_vehicle_data, sizeof(serialized_vehicle_data), 0 /* packed */);
    capn_free(&c);
}