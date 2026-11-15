#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_wifi_sensing.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSI_UDP_BROADCAST_PORT 5005
#define CSI_UDP_MAX_PAYLOAD 2048
#define CSI_UDP_MAX_PEERS 3

typedef struct {
    const char *name;
    uint8_t mac[6];
} csi_udp_peer_t;

typedef struct {
    esp_wifi_sensing_fsm_handle_t fsm;
    const csi_udp_peer_t *peers;
    size_t peer_count;
    const char *broadcast_address;
    uint16_t port;
} csi_udp_broadcast_config_t;

esp_err_t csi_udp_broadcast_start(const csi_udp_broadcast_config_t *config);

#ifdef __cplusplus
}
#endif
