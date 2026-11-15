# FreeDetector: Device-Free Occupancy Detection with Commodity WiFi

- **Type:** occupancy-detection paper
- **Source:** [IEEE Xplore](https://ieeexplore.ieee.org/abstract/document/8011040)
- **Use here:** occupancy as a separate objective from motion

## Algorithm findings

Occupancy detection asks whether a person is present, including cases where
the person may not be moving. This is different from a motion detector that
responds mainly to short-term CSI changes. Occupancy systems therefore need
longer windows, a stable empty-scene reference, or a classifier trained with
empty and occupied examples.

## Relevance and limitations

The ARMv7 service should expose `presence` and `motion` separately. A single
frame-to-frame jitter threshold is insufficient to claim reliable static-human
presence detection.
