#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include "header.h"
#include "driver/uart.h"
#include "proto_control.h"
extern TaskHandle_t handle_uart;

void uart_start(vehicle_data_t* vehicle_state_data);
void uart_init(vehicle_data_t* vehicle_state_data);

#endif // !UART_CONTROL_H