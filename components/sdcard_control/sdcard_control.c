#include "sdcard_control.h"

TaskHandle_t handle_sdcard = NULL;
const char* TAG_SD_CARD = "SD_CARD";

esp_err_t sd_card_init()
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0,
    };
    sdmmc_card_t* card;
    const char mount_point[] = CONFIG_MOUNT_POINT;
    ESP_LOGI(TAG_SD_CARD, "Initializing SD card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_PIN_NUM_MOSI,
        .miso_io_num = CONFIG_PIN_NUM_MISO,
        .sclk_io_num = CONFIG_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SD_CARD, "Failed to initialize bus.");
        return ret;
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG_SD_CARD, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_SD_CARD, "Failed to mount filesystem. ");
        }
        else
        {
            ESP_LOGE(TAG_SD_CARD, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG_SD_CARD, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    return ret;
}

void sd_card_write(void* arg)
{
    const char* file_path = CONFIG_MOUNT_POINT "/SD_data.txt";
    ESP_LOGI(TAG_WIFI, "Opening file %s", CONFIG_MOUNT_POINT);
    FILE* f = fopen(file_path, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG_WIFI, "Failed to open file for writing");
        return;
    }
    ESP_LOG_BUFFER_HEXDUMP(TAG_SD_CARD, &vehicle_state_data, sizeof(vehicle_state_data), 2);

    fclose(f);
    ESP_LOGI(TAG_SD_CARD, "File written");
    vTaskDelete(handle_sdcard);
}
