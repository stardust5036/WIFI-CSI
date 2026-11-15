# WIFI_CSI_based_HAR

- **Repository:** [Retsediv/WIFI_CSI_based_HAR](https://github.com/Retsediv/WIFI_CSI_based_HAR)
- **Observed popularity:** 215 stars, 34 forks
- **Source paths checked:** `data_retrieval/csi_extraction/read_csi.py`,
  `csi_calibration.py`, `model/models/InceptionTime.py`

## Raw CSI -> result

```text
binary CSI record
  -> parse big-endian metadata and 10-bit packed signed I/Q
  -> complex subcarrier matrix
  -> unwrap phase, remove phase slope/intercept
  -> normalize amplitude with RSSI
  -> fixed time-series sample
  -> InceptionTime/LSTM classifier
  -> activity class
```

This is an activity-recognition project, not a binary presence detector.
`algorithm.py` contains a dependency-free parser/calibration reference; the
PyTorch model itself requires the repository's training environment.
