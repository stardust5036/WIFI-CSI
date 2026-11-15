# Temporal vs Spectral Resolution in CSI Reporting for Wi-Fi ISAC

- **Published:** 16 October 2026
- **Source:** [arXiv:2610.23422](https://arxiv.org/abs/2610.23422)

## Algorithm

The paper compares reporting policies at the same average report rate:

- periodic or random reporting;
- IEEE 802.11bf threshold-triggered reporting;
- current-sample payload versus mean CSI accumulated since the previous
  report;
- uniform subcarrier decimation versus block-mean grouping.

The receiver reconstructs missing time samples with zero-order hold and uses
a CNN-GRU with temporal-pyramid pooling. Cost is measured using the 802.11bf
CSI report size. Thresholds are tuned on unlabeled CSI to reach a target
report rate rather than tuned from activity labels.

## Relevance

For the ESP32 UDP design, this supports protecting temporal resolution before
aggressively reducing CSI frames. Packet loss, sequence gaps, and timestamps
should be logged before reducing the raw stream. The paper is about feedback
cost and HAR, not a replacement for the binary presence state machine.
