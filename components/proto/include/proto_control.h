#ifndef PROTO_CONTROL_H
#define PROTO_CONTROL_H

#include "header.h"
#include "vehicle_data.capnp.h"

typedef struct VehicleData vehicle_data_t;

void serialize_vehicle_data(vehicle_data_t* vehicle_state_data, uint8_t* serialized_vehicle_data, ssize_t* buffer_len);

#endif // !PROTO_CONTROL_H