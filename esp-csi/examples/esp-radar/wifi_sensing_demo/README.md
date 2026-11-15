# ESP Wi-Fi Sensing Demo

This directory contains a self-contained ESP-IDF demo used to exercise the
`esp_wifi_sensing` component in a realistic bring-up flow.

The example combines:

- Wi-Fi STA connection and sensing FSM startup
- Three monitored channels: the connected AP plus two fixed peer MAC addresses
- A status LED policy for quick visual feedback
- UDP broadcast of raw CSI data and firmware sensing results

Each UDP packet uses the `CSIU` magic and version `1`. `type=1` contains the
raw CSI bytes, source MAC, sequence, data type, and CSI lengths. `type=2`
contains the firmware motion/presence result and jitter/wander diagnostics.
Both packet types use UDP port `5005` and the broadcast address
`255.255.255.255`.

The example follows the same public workflow recommended by the component:

1. Create the FSM with `esp_wifi_sensing_fsm_create()`.
2. Add the AP channel and two extra peer channels.
3. Register ACTIVE / INACTIVE callbacks.
4. Start the FSM with `esp_wifi_sensing_fsm_control(..., ESP_WIFI_SENSING_FSM_CTRL_START, NULL)`.
5. Optionally keep CSI traffic flowing with `esp_wifi_sensing_fsm_ping_router_start()`.

## Directory Layout

- `main/app_main.c`: example entry point and sensing FSM wiring
- `main/led_control.*`: LED backend and state mapping
- `main/csi_udp_broadcast.*`: raw CSI and firmware-result UDP broadcaster
- `services/csi_udp_service`: ARMv7 Rust receiver and independent CSI algorithm

## Build and Run

Open the `wifi_sensing_demo` directory as an ESP-IDF project, then build and
flash it in the usual way:

```bash
idf.py set-target <chip>
idf.py flash monitor
```

The example uses `protocol_examples_common` for Wi-Fi provisioning, so configure
your Wi-Fi credentials before flashing if your local workflow requires it.

## ARMv7 receiver

Build and run `services/csi_udp_service` on the ARMv7 host. It receives both
packet types, calculates an independent motion/presence result from each raw
CSI frame, and prints it beside the most recent firmware result.

## Demo Behavior

- The AP BSSID channel is treated as the primary channel for LED feedback.
- The LED blinks while Wi-Fi is disconnected.
- When the AP channel becomes ACTIVE, the LED turns fully on.
- When the AP channel returns to INACTIVE, the LED fades out.
- The two fixed peer channels remain visible in logs as extra sensing references.

