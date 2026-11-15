# wifi-ghost

- **Repository:** [heyfinal/wifi-ghost](https://github.com/heyfinal/wifi-ghost)
- **Found by:** GitHub repository search, checked November 2026

## Scope

The project uses ESP32-WROOM transmitter/receiver nodes and a Python
aggregator. Its documented motion layer uses selected subcarriers, a
per-packet inter-subcarrier standard deviation, a sliding window, a Hampel
filter, adaptive P95 baseline thresholding, and 3-hit/5-hit hysteresis.
Respiration is a separate long-window FFT layer.

## Relevance

This is a useful directly comparable open implementation. Its documented
subcarrier list, 10-second calibration, and adaptive threshold are
project-specific settings, not universal CSI constants. The current service
can add this as an optional algorithm after raw packet compatibility is
verified.
