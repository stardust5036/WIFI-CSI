#include "csi_udp_broadcast.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_radar.h"

#define CSI_UDP_MAGIC "CSIU"
#define CSI_UDP_VERSION 1
#define CSI_UDP_TYPE_RAW 1
#define CSI_UDP_TYPE_RESULT 2
#define CSI_UDP_QUEUE_LENGTH 8
#define CSI_UDP_RESULT_PERIOD_MS 100

static const char *TAG = "csi_udp";

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

typedef struct {
    csi_udp_header_t header;
    uint8_t payload[CSI_UDP_MAX_PAYLOAD];
} csi_udp_packet_t;

typedef struct {
    int socket;
    struct sockaddr_in destination;
    QueueHandle_t queue;
    TaskHandle_t task;
    esp_wifi_sensing_fsm_handle_t fsm;
    csi_udp_peer_t peers[CSI_UDP_MAX_PEERS];
    size_t peer_count;
    uint32_t result_sequence;
} csi_udp_context_t;

static csi_udp_context_t s_ctx;

static void csi_udp_send_packet(csi_udp_packet_t *packet)
{
    size_t size = sizeof(packet->header) + packet->header.payload_len;
    int sent = sendto(s_ctx.socket, packet, size, 0,
                      (const struct sockaddr *)&s_ctx.destination,
                      sizeof(s_ctx.destination));
    if (sent < 0) {
        ESP_LOGW(TAG, "sendto failed: errno=%d", errno);
    }
}

static void csi_udp_publish_results(void)
{
    for (size_t i = 0; i < s_ctx.peer_count; ++i) {
        esp_wifi_sensing_fsm_channel_diag_t diag = {0};
        if (esp_wifi_sensing_fsm_get_channel_diag(s_ctx.fsm, s_ctx.peers[i].mac, &diag) != ESP_OK) {
            continue;
        }

        csi_udp_packet_t packet = {0};
        memcpy(packet.header.magic, CSI_UDP_MAGIC, sizeof(packet.header.magic));
        packet.header.version = CSI_UDP_VERSION;
        packet.header.type = CSI_UDP_TYPE_RESULT;
        packet.header.header_len = sizeof(packet.header);
        packet.header.sequence = s_ctx.result_sequence++;
        packet.header.timestamp_us = (uint32_t)(esp_timer_get_time() & UINT32_MAX);
        memcpy(packet.header.source_mac, s_ctx.peers[i].mac, sizeof(packet.header.source_mac));
        packet.header.motion_active =
            (diag.state == ESP_WIFI_SENSING_FSM_PROCESS_ACTIVE) ? 1 : 0;
        packet.header.presence_active = diag.presence_someone_status ? 1 : 0;
        packet.header.jitter = diag.jitter_value;
        packet.header.wander = diag.wander_value;
        packet.header.payload_len = 0;
        csi_udp_send_packet(&packet);
    }
}

static void csi_udp_task(void *arg)
{
    (void)arg;
    TickType_t last_result = xTaskGetTickCount();
    csi_udp_packet_t packet = {0};

    while (true) {
        if (xQueueReceive(s_ctx.queue, &packet, pdMS_TO_TICKS(20)) == pdTRUE) {
            csi_udp_send_packet(&packet);
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - last_result) >= pdMS_TO_TICKS(CSI_UDP_RESULT_PERIOD_MS)) {
            csi_udp_publish_results();
            last_result = now;
        }
    }
}

static void csi_udp_filtered_csi_cb(void *ctx, const wifi_csi_filtered_info_t *info)
{
    (void)ctx;
    if (!info || !info->raw_data || info->raw_len == 0) {
        return;
    }

    uint16_t payload_len = info->raw_len;
    if (payload_len > CSI_UDP_MAX_PAYLOAD) {
        payload_len = CSI_UDP_MAX_PAYLOAD;
    }

    csi_udp_packet_t packet = {0};
    memcpy(packet.header.magic, CSI_UDP_MAGIC, sizeof(packet.header.magic));
    packet.header.version = CSI_UDP_VERSION;
    packet.header.type = CSI_UDP_TYPE_RAW;
    packet.header.header_len = sizeof(packet.header);
    packet.header.payload_len = payload_len;
    packet.header.sequence = info->seq_id;
    packet.header.timestamp_us = info->rx_ctrl_info.timestamp;
    memcpy(packet.header.source_mac, info->mac, sizeof(packet.header.source_mac));
    packet.header.data_type = (uint8_t)info->data_type;
    packet.header.raw_len = info->raw_len;
    packet.header.valid_len = info->valid_len;
    memcpy(packet.payload, info->raw_data, payload_len);

    if (xQueueSend(s_ctx.queue, &packet, 0) != pdTRUE) {
        ESP_LOGD(TAG, "CSI packet dropped because the UDP queue is full");
    }
}

esp_err_t csi_udp_broadcast_start(const csi_udp_broadcast_config_t *config)
{
    if (!config || !config->fsm || !config->peers ||
            config->peer_count == 0 || config->peer_count > CSI_UDP_MAX_PEERS) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.fsm = config->fsm;
    s_ctx.peer_count = config->peer_count;
    memcpy(s_ctx.peers, config->peers, config->peer_count * sizeof(s_ctx.peers[0]));
    s_ctx.queue = xQueueCreate(CSI_UDP_QUEUE_LENGTH, sizeof(csi_udp_packet_t));
    if (!s_ctx.queue) {
        return ESP_ERR_NO_MEM;
    }

    s_ctx.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s_ctx.socket < 0) {
        return ESP_FAIL;
    }

    int broadcast = 1;
    if (setsockopt(s_ctx.socket, SOL_SOCKET, SO_BROADCAST,
                   &broadcast, sizeof(broadcast)) != 0) {
        close(s_ctx.socket);
        s_ctx.socket = -1;
        return ESP_FAIL;
    }

    s_ctx.destination.sin_family = AF_INET;
    s_ctx.destination.sin_port = htons(config->port ? config->port : CSI_UDP_BROADCAST_PORT);
    s_ctx.destination.sin_addr.s_addr = inet_addr(
        config->broadcast_address ? config->broadcast_address : "255.255.255.255");

    esp_radar_config_t radar_config = {0};
    esp_err_t err = esp_radar_get_config(&radar_config);
    if (err != ESP_OK) {
        close(s_ctx.socket);
        s_ctx.socket = -1;
        return err;
    }
    radar_config.csi_config.csi_filtered_cb = csi_udp_filtered_csi_cb;
    radar_config.csi_config.csi_filtered_cb_ctx = &s_ctx;
    err = esp_radar_change_config(&radar_config);
    if (err != ESP_OK) {
        close(s_ctx.socket);
        s_ctx.socket = -1;
        return err;
    }

    if (xTaskCreate(csi_udp_task, "csi_udp", 4096, NULL, 5, &s_ctx.task) != pdPASS) {
        close(s_ctx.socket);
        s_ctx.socket = -1;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "UDP broadcast started: %s:%u",
             config->broadcast_address ? config->broadcast_address : "255.255.255.255",
             config->port ? config->port : CSI_UDP_BROADCAST_PORT);
    return ESP_OK;
}
