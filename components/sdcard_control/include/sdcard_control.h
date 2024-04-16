#ifndef SDCARD_CONTROL_H
#define SDCARD_CONTROL_H

#include "header.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

extern TaskHandle_t handle_sdcard;

esp_err_t sd_card_init();
void sd_card_write(void* arg);

#endif // !SDCARD_CONTROL_H