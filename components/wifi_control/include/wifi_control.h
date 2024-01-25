#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H
#include "header.h"
#include "esp_netif_types.h"
#include "esp_wifi_types.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "nvs_flash.h"


extern bool wifi_ready;
#endif // !WIFI_CONTROL_H

void wifi_init();
void wifi_start();