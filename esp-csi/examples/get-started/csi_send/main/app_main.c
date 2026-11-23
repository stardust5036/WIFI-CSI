/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "lwip/inet.h"
#include "ping/ping_sock.h"

#define CONFIG_CSI_AP_SSID        "ESP-CSI-AP"
#define CONFIG_CSI_AP_PASSWORD    ""
#define CONFIG_SEND_FREQUENCY     100

#define WIFI_CONNECTED_BIT        BIT0

static const char *TAG = "csi_send";
static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGW(TAG, "Disconnected from CSI AP, reconnecting");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, CONFIG_CSI_AP_SSID,
            sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, CONFIG_CSI_AP_PASSWORD,
            sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                               wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                               wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               wifi_event_handler, NULL));

    wifi_init();

    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                        pdFALSE, pdFALSE, portMAX_DELAY);

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.count = 0;
    ping_config.interval_ms = 1000 / CONFIG_SEND_FREQUENCY;
    ping_config.task_stack_size = 3072;
    ping_config.data_size = 1;

    esp_netif_ip_info_t local_ip;
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    ESP_ERROR_CHECK(esp_netif_get_ip_info(sta_netif, &local_ip));
    ping_config.target_addr.u_addr.ip4.addr = ip4_addr_get_u32(&local_ip.gw);
    ping_config.target_addr.type = ESP_IPADDR_TYPE_V4;

    esp_ping_handle_t ping_handle = NULL;
    esp_ping_callbacks_t ping_callbacks = {0};
    ESP_ERROR_CHECK(esp_ping_new_session(&ping_config, &ping_callbacks, &ping_handle));
    ESP_ERROR_CHECK(esp_ping_start(ping_handle));

    ESP_LOGI(TAG, "================ CSI SEND ================");
    ESP_LOGI(TAG, "Connected to CSI AP %s, ping gateway " IPSTR " at %d Hz",
             CONFIG_CSI_AP_SSID, IP2STR(&local_ip.gw), CONFIG_SEND_FREQUENCY);
}
