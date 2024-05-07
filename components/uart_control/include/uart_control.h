#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include "header.h"
#include "driver/uart.h"
#include "proto_control.h"

extern TaskHandle_t handle_uart;

void uart_start();
void uart_init(void (*on_read_callback)(uint8_t*, uint32_t));
void uart_send_data(uint8_t* data, uint32_t size);

#endif // !UART_CONTROL_H