/*
 * SPDX-FileCopyrightText: 2025-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* Get Start Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"

#include "esp_mac.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_csi_gain_ctrl.h"
#include "esp_event.h"
#include "esp_timer.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"
#define CONFIG_AP_SSID                     "ESP-CSI-AP"
#define CONFIG_AP_PASSWORD                 ""
#define CONFIG_CSI_AP_MAX_CONNECTION       4
#define CONFIG_FORCE_GAIN                   0

#if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C61
#define CSI_FORCE_LLTF                      0
#endif

#if CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61
#define CONFIG_GAIN_CONTROL                 1
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
#define ESP_IF_WIFI_STA ESP_MAC_WIFI_STA
#endif

static const char *TAG = "csi_recv";

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t s_wifi_event_group;
static uint8_t s_send_sta_mac[6];
static int s_udp_socket = -1;
static struct sockaddr_in s_udp_destination;
static uint32_t s_udp_sequence = 0;

typedef struct __attribute__((packed)) {
    char magic[4];
    uint8_t version;
    uint8_t type;
    uint16_t header_len;
    uint16_t payload_len;
    uint16_t reserved;
    uint32_t sequence;
    uint32_t timestamp_us;
    uint8_t source_mac[6];
    uint8_t data_type;
    uint8_t motion_active;
    uint8_t presence_active;
    uint8_t reserved_flags;
    float jitter;
    float wander;
    uint16_t raw_len;
    uint16_t valid_len;
} csi_udp_header_t;

static void wifi_init()
{
    if (strlen(CONFIG_CSI_PUBLIC_WIFI_SSID) == 0) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    wifi_config_t ap_config = {
        .ap = {
            .ssid = CONFIG_AP_SSID,
            .ssid_len = sizeof(CONFIG_AP_SSID) - 1,
            .password = CONFIG_AP_PASSWORD,
            .channel = CONFIG_CSI_AP_CHANNEL,
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = CONFIG_CSI_AP_MAX_CONNECTION,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    wifi_config_t sta_config = {0};
    strlcpy((char *)sta_config.sta.ssid, CONFIG_CSI_PUBLIC_WIFI_SSID,
            sizeof(sta_config.sta.ssid));
    strlcpy((char *)sta_config.sta.password, CONFIG_CSI_PUBLIC_WIFI_PASSWORD,
            sizeof(sta_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGW(TAG, "Wi-Fi disconnected, reconnecting");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        memcpy(s_send_sta_mac, event->mac, sizeof(s_send_sta_mac));
        ESP_LOGI(TAG, "CSI sender connected: " MACSTR, MAC2STR(s_send_sta_mac));
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t csi_udp_init(void)
{
    s_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s_udp_socket < 0) {
        return ESP_FAIL;
    }

    if (strcmp(CONFIG_CSI_UDP_TARGET_IP, "255.255.255.255") == 0) {
        int broadcast = 1;
        if (setsockopt(s_udp_socket, SOL_SOCKET, SO_BROADCAST,
                       &broadcast, sizeof(broadcast)) != 0) {
            close(s_udp_socket);
            s_udp_socket = -1;
            return ESP_FAIL;
        }
        s_udp_destination.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        ESP_LOGI(TAG, "UDP CSI broadcast started on public Wi-Fi: 255.255.255.255:%d",
                 CONFIG_CSI_UDP_PORT);
    } else {
        struct in_addr target_addr;
        if (inet_pton(AF_INET, CONFIG_CSI_UDP_TARGET_IP, &target_addr) != 1) {
            close(s_udp_socket);
            s_udp_socket = -1;
            ESP_LOGE(TAG, "Invalid UDP target IP: %s", CONFIG_CSI_UDP_TARGET_IP);
            return ESP_ERR_INVALID_ARG;
        }
        s_udp_destination.sin_addr = target_addr;
        ESP_LOGI(TAG, "UDP CSI unicast started on public Wi-Fi: %s:%d",
                 CONFIG_CSI_UDP_TARGET_IP, CONFIG_CSI_UDP_PORT);
    }

    s_udp_destination.sin_family = AF_INET;
    s_udp_destination.sin_port = htons(CONFIG_CSI_UDP_PORT);
    return ESP_OK;
}

static void csi_udp_broadcast_raw(const wifi_csi_info_t *info)
{
    if (s_udp_socket < 0 || !info || !info->buf || info->len <= 0) {
        return;
    }

    uint16_t raw_len = info->len;
    uint16_t payload_len = raw_len > 2048 ? 2048 : raw_len;

    uint8_t packet[sizeof(csi_udp_header_t) + 2048];
    csi_udp_header_t *header = (csi_udp_header_t *)packet;
    memset(header, 0, sizeof(*header));
    memcpy(header->magic, "CSIU", 4);
    header->version = 1;
    header->type = 1;
    header->header_len = sizeof(*header);
    header->payload_len = payload_len;
    header->sequence = s_udp_sequence++;
    header->timestamp_us = info->rx_ctrl.timestamp;
    memcpy(header->source_mac, s_send_sta_mac, sizeof(header->source_mac));
#if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61
    header->data_type = 1;
#else
    header->data_type = 0;
#endif
    header->raw_len = raw_len;
    header->valid_len = payload_len;
    memcpy(packet + sizeof(*header), info->buf, payload_len);

    int sent = sendto(s_udp_socket, packet, sizeof(*header) + payload_len, 0,
                      (const struct sockaddr *)&s_udp_destination,
                      sizeof(s_udp_destination));
    if (sent < 0) {
        ESP_LOGW(TAG, "UDP CSI send failed: errno=%d", errno);
    }
}

static void wifi_csi_rx_cb(void *ctx, wifi_csi_info_t *info)
{
    if (!info || !info->buf) {
        ESP_LOGW(TAG, "<%s> wifi_csi_cb", esp_err_to_name(ESP_ERR_INVALID_ARG));
        return;
    }

    if (s_send_sta_mac[0] == 0 && s_send_sta_mac[1] == 0 &&
        s_send_sta_mac[2] == 0 && s_send_sta_mac[3] == 0 &&
        s_send_sta_mac[4] == 0 && s_send_sta_mac[5] == 0) {
        return;
    }
    if (memcmp(info->mac, s_send_sta_mac, 6)) {
        return;
    }

    const wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;
    static int s_count = 0;
    float compensate_gain = 1.0f;
    static uint8_t agc_gain = 0;
    static int8_t fft_gain = 0;
#if CONFIG_GAIN_CONTROL
    static uint8_t agc_gain_baseline = 0;
    static int8_t fft_gain_baseline = 0;
    esp_csi_gain_ctrl_get_rx_gain(rx_ctrl, &agc_gain, &fft_gain);
    if (s_count < 100) {
        esp_csi_gain_ctrl_record_rx_gain(agc_gain, fft_gain);
    } else if (s_count == 100) {
        esp_csi_gain_ctrl_get_rx_gain_baseline(&agc_gain_baseline, &fft_gain_baseline);
#if CONFIG_FORCE_GAIN
        esp_csi_gain_ctrl_set_rx_force_gain(agc_gain_baseline, fft_gain_baseline);
        ESP_LOGD(TAG, "fft_force %d, agc_force %d", fft_gain_baseline, agc_gain_baseline);
#endif
    }
    esp_csi_gain_ctrl_get_gain_compensation(&compensate_gain, agc_gain, fft_gain);
    ESP_LOGI(TAG, "compensate_gain %f, agc_gain %d, fft_gain %d", compensate_gain, agc_gain, fft_gain);
#endif

    csi_udp_broadcast_raw(info);
    uint32_t rx_id = s_count;
#if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61
    if (!s_count) {
        ESP_LOGI(TAG, "================ CSI RECV ================");
        ets_printf("type,seq,mac,rssi,rate,noise_floor,fft_gain,agc_gain,channel,local_timestamp,sig_len,rx_format,len,first_word,data\n");
    }

    ets_printf("CSI_DATA,%d," MACSTR ",%d,%d,%d,%d,%d,%d,%d,%d,%d",
               rx_id, MAC2STR(info->mac), rx_ctrl->rssi, rx_ctrl->rate,
               rx_ctrl->noise_floor, fft_gain, agc_gain,  rx_ctrl->channel,
               rx_ctrl->timestamp, rx_ctrl->sig_len, rx_ctrl->cur_bb_format);
#else
    if (!s_count) {
        ESP_LOGI(TAG, "================ CSI RECV ================");
        ets_printf("type,id,mac,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_format,len,first_word,data\n");
    }

    ets_printf("CSI_DATA,%d," MACSTR ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
               rx_id, MAC2STR(info->mac), rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
               rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
               rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
               rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
               rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->sig_mode);

#endif
#if (CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C61) && CSI_FORCE_LLTF
    int16_t csi = ((int16_t)(((((uint16_t)info->buf[1]) << 8) | info->buf[0]) << 4) >> 4);
    ets_printf(",%d,%d,\"[%d", (info->len - 2) / 2, info->first_word_invalid, (int16_t)(compensate_gain * csi));
    for (int i = 2; i < (info->len - 2); i += 2) {
        csi = ((int16_t)(((((uint16_t)info->buf[i + 1]) << 8) | info->buf[i]) << 4) >> 4);
        ets_printf(",%d", (int16_t)(compensate_gain * csi));
    }
#else
    ets_printf(",%d,%d,\"[%d", info->len, info->first_word_invalid, (int16_t)(compensate_gain * info->buf[0]));
    for (int i = 1; i < info->len; i++) {
        ets_printf(",%d", (int16_t)(compensate_gain * info->buf[i]));
    }
#endif
    ets_printf("]\"\n");
    s_count++;
}

static void wifi_csi_init()
{
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

    /**< default config */
#if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C61
    wifi_csi_config_t csi_config = {
        .enable                   = true,
        .acquire_csi_legacy       = true,
        .acquire_csi_force_lltf   = CSI_FORCE_LLTF,
        .acquire_csi_ht20         = true,
        .acquire_csi_ht40         = true,
        .acquire_csi_vht          = false,
        .acquire_csi_su           = false,
        .acquire_csi_mu           = false,
        .acquire_csi_dcm          = false,
        .acquire_csi_beamformed   = false,
        .acquire_csi_he_stbc_mode = 2,
        .val_scale_cfg            = 0,
        .dump_ack_en              = false,
        .reserved                 = false
    };
#elif CONFIG_IDF_TARGET_ESP32C6
    wifi_csi_config_t csi_config = {
        .enable                 = true,
        .acquire_csi_legacy     = true,
        .acquire_csi_ht20       = true,
        .acquire_csi_ht40       = true,
        .acquire_csi_su         = true,
        .acquire_csi_mu         = true,
        .acquire_csi_dcm        = true,
        .acquire_csi_beamformed = true,
        .acquire_csi_he_stbc    = 2,
        .val_scale_cfg          = false,
        .dump_ack_en            = false,
        .reserved               = false
    };
#else
    wifi_csi_config_t csi_config = {
        .lltf_en           = true,
        .htltf_en          = true,
        .stbc_htltf2_en    = true,
        .ltf_merge_en      = true,
        .channel_filter_en = true,
        .manu_scale        = false,
        .shift             = false,
    };
#endif
    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(wifi_csi_rx_cb, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));
}

void app_main()
{
    /**
     * @brief Initialize NVS
     */
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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED,
                                               wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               wifi_event_handler, NULL));

    wifi_init();

    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                        pdFALSE, pdFALSE, portMAX_DELAY);

    ESP_LOGI(TAG, "connected to public Wi-Fi SSID: %s",
             CONFIG_CSI_PUBLIC_WIFI_SSID);
    ESP_LOGI(TAG, "CSI SoftAP SSID: %s", CONFIG_AP_SSID);

    ESP_ERROR_CHECK(csi_udp_init());
    wifi_csi_init();
}
