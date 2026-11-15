# ESPCAL

- **Repository:** [xMatrix-Lab/ESPCAL](https://github.com/xMatrix-Lab/ESPCAL)
- **Observed popularity:** 40 stars, 8 forks
- **Source checked:** `csi_system/algorithms/`

## Raw CSI -> result

```text
multi-antenna CSI
  -> CIR synchronization / calibration
  -> RSSI weighting
  -> iterative/eigenvector filtering
  -> antenna/subcarrier covariance
  -> MUSIC 1D/2D spectrum
  -> angle-of-arrival / localization estimate
```

This is an array-processing project, not a motion-presence FSM. It is
valuable for phase-coherent spatial features but cannot be directly mixed
with the current single-peer amplitude detector.
