# WiFi human presence detection system

- **Repository:** [vincenzo-afk/WIFI-based-human-presence-detection-system](https://github.com/vincenzo-afk/WIFI-based-human-presence-detection-system)
- **Observed popularity:** 46 stars, 2 forks
- **Source checked:** `backend/csi_engine.py`, `backend/detector.py`

## Raw CSI -> result

The project replays CSI records, maintains an empty-scene baseline, computes
windowed deviations, and sends presence/zone/activity results to a dashboard.
The exact model path is configuration-dependent; this summary must not be
treated as a universal threshold.
