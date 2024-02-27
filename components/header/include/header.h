#ifndef HEADER_H
#define HEADER_H


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"




/////////////TAGS
extern const char *TAG_WIFI;
extern const char *TAG_MQTT;
extern const char *TAG_UART;
extern const char *TAG_SD_CARD;



///////////// Data structure
typedef struct{  
  union {
    struct { // TODO remove bitfield since it won't be used here anyway
      uint8_t is_emergency:1;
      uint8_t is_hydrogen_leaking:1;
      uint8_t is_sc_relay_closed:1;
      uint8_t vehicle_is_speed_button_pressed:1;
      uint8_t vehicle_is_half_speed_button_pressed:1;
      uint8_t hydrogen_cell_button_state:2; // Split into two variables?
      uint8_t is_super_capacitor_button_pressed:1;
    };
    uint8_t logic_state; // 4b
  };
  float fc_current; // 8b
  float fc_sc_current; // 12b
  float sc_motor_current; // 16b
  float fc_voltage; // 20b
  float sc_voltage; // 24b
  float hydrogen_sensor_voltage; // 28b
  float fuel_cell_temperature; // 32b
  uint16_t fan_rpm; // 36b
  float vehicle_speed; // 40b
  uint8_t motor_pwm; // 44b
  float hydrogen_pressure; //48b
}vehicle_state_frame_t; 

extern vehicle_state_frame_t vehicle_state_data;
#endif // !HEADER_H