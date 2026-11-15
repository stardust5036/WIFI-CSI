# baby-monitor-wifi-csi

- **Repository:** [mohosy/baby-monitor-wifi-csi](https://github.com/mohosy/baby-monitor-wifi-csi)
- **Observed popularity:** 24 stars, 1 fork
- **Source checked:** `backend/processing/preprocessor.py`,
  `apnea_detector.py`, firmware UDP streamer

## Raw CSI -> result

```text
ESP32 CSI
  -> UDP stream
  -> parser and preprocessing
  -> amplitude time series
  -> breathing-band filtering
  -> apnea/breath absence timer
  -> alert result
```

This project targets respiration/apnea rather than general motion. Its
thresholds and sample-rate assumptions must not be copied to the current
presence service.
