#ifndef UART_CONTROL_H
#define UART_CONTROL_H

#include "header.h"
#include "driver/uart.h"

extern TaskHandle_t handle_uart;

void uart_start();
void uart_init(void* arg);

#endif // !UART_CONTROL_H