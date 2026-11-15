# whofi

- **Repository:** [jedarden-org/whofi](https://github.com/jedarden-org/whofi)
- **Observed popularity:** 102 stars, 11 forks
- **Source checked:** `csi-firmware/components/csi_collector/src/csi_filter.c`

## Raw CSI -> result

```text
ESP32 CSI callback
  -> bounded collector buffer
  -> configurable validity/range filtering
  -> smoothing and frame-rate control
  -> amplitude/phase feature stream
  -> RSSI/CSI presence or positioning estimator
  -> calibrated location/presence result
```

The repository contains several firmware examples, so there is no single
universal classifier. The filter component is verified; the final estimator
depends on the selected example.
