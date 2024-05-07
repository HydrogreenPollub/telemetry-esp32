#ifndef PROTO_CONTROL_H
#define PROTO_CONTROL_H

#include "header.h"
#include "ts_data.capnp.h"
#include "mt_data.capnp.h"

typedef struct MTData master_telemetry_data_t;
typedef struct TSData telemetry_server_data_t;

int serialize_telemetry_server_data(telemetry_server_data_t ts_data, uint8_t* output_data, ssize_t* output_data_len);

int deserialize_master_telemetry_data(master_telemetry_data_t* ms_data, uint8_t* input_data, ssize_t* input_data_len);

#endif // !PROTO_CONTROL_H