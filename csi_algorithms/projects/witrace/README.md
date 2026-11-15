# WiTrace

- **Repository:** [mizhab-as/WiTrace](https://github.com/mizhab-as/WiTrace)
- **Observed popularity:** 10 stars, 4 forks
- **Source checked:** `DETECTION_WINDOW_5SEC.md`, firmware receiver

## Raw CSI -> result

```text
ESP32 CSI receiver
  -> amplitude feature extraction
  -> 5-second detection window
  -> baseline/variance and presence features
  -> occupancy classifier
  -> presence/occupancy result
```

The repository includes explicit detection-window design notes. Exact model
selection remains project configuration.
