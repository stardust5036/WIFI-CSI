# Time-Selective RNN for Device-Free Multiroom Human Presence Detection Using WiFi CSI

- **Type:** presence-detection paper
- **Source:** [IEEE Xplore](https://ieeexplore.ieee.org/abstract/document/10379149)
- **Use here:** temporal modeling and multi-room generalization

## Algorithm findings

The work uses CSI time series and a recurrent model with temporal selection for
multi-room presence detection. The key design lesson is that the useful
evidence is distributed across time, not necessarily in one sample.

## Relevance and limitations

An RNN is too expensive for the current no-dependency baseline, but the
reference detector mirrors the temporal principle with activity smoothing,
long-term baseline deviation, hysteresis, and debounce. Model performance
cannot be transferred to ESP32 without matching data and evaluation.
