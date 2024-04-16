#include "proto_control.h"

ssize_t serialized_vehicle_data = 0;
ssize_t buffer_len = 0;

void serialize_vehicle_data()
{
    struct capn c;
    capn_init_malloc(&c);
    capn_ptr cr = capn_root(&c);
    struct capn_segment* cs = cr.seg;
    VehicleData_ptr ptr = new_VehicleData(cs);
    write_VehicleData(&vehicle_state_data, ptr);
    int setp_ret = capn_setp(capn_root(&c), 0, ptr.p);
    // ASSERT_EQ(0, setp_ret);
    buffer_len = capn_write_mem(&c, buf, sizeof(buf), 0 /* packed */);
    capn_free(&c);
}