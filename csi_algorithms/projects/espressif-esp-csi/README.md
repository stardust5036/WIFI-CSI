# Espressif ESP-CSI / esp-radar

- **Source:** [espressif/esp-csi](https://github.com/espressif/esp-csi)
- **Example:** [wifi_sensing_demo](https://github.com/espressif/esp-csi/tree/master/examples/esp-radar/wifi_sensing_demo)

## Observable project behavior

The demo creates `esp_wifi_sensing_fsm`, adds the AP BSSID and peer channels,
registers ACTIVE/INACTIVE callbacks, and can start router-ping-assisted CSI
sampling. Public diagnostics expose `jitter_value`, smoothing, enter/exit
levels, state, and training/init status. The FSM has confirmation and
state-transition behavior.

The public example does **not** prove that its closed motion decoder uses CWT,
PCA, SVM, or a neural network. The exact `esp_radar_motion_dec` implementation
is a precompiled component and must not be conflated with the reference
algorithm in `reference/`.

## Deployment implication

The current firmware broadcasts raw CSI and a separate firmware result. The
Rust service calculates its own result, so disagreements are expected until
the service is calibrated and its features are made equivalent to the firmware
decoder.
