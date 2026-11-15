# A Machine Learning Approach to Passive Human Motion Detection Using WiFi Measurements From Commodity IoT Devices

- **Type:** motion-detection paper
- **Source:** [IEEE Xplore](https://ieeexplore.ieee.org/abstract/document/10122883)
- **Use here:** low-cost IoT motion detection

## Algorithm findings

Commodity IoT measurements can support passive motion detection when a window
of signal-derived features is used instead of a single RSSI/CSI value. A
supervised classifier can replace hand-tuned thresholds when labeled data is
available.

## Relevance and limitations

The current reference implementation intentionally stays rule-based so it can
run without a model runtime on ARMv7. A future trained classifier should be
compared against the same held-out rooms and people, not only random adjacent
windows.
