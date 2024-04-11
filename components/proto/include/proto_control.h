#ifndef PROTO_CONTROL_H
#define PROTO_CONTROL_H
#include "header.h"
#include "vehicle_data.capnp.h"

void serialize_vehicle_data();

extern ssize_t serialized_vehicle_data;
extern ssize_t buffer_len;
extern uint8_t buf[4096];

#endif // !PROTO_CONTROL_H