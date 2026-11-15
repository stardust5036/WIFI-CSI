# A Proof-of-Concept Study of Wi-Fi CSI-Based Fall Detection Using ESP32 Devices

- **Published:** 11 November 2026
- **Source:** [arXiv:2611.15532](https://arxiv.org/abs/2611.15532)

## Algorithm

The acquisition uses two ESP32 boards separated by about 3.5 m, with the
activity performed between them. The pipeline is:

```text
raw CSI IQ
  -> per-subcarrier amplitude sqrt(I^2 + Q^2)
  -> either 10 Hz 4th-order Butterworth low-pass
     or level-2 Symlet-10 DWT denoising
  -> per-record z-score normalization
  -> five-second fixed-length segment
  -> wavelet-scattering time-frequency coefficients
  -> binary LogitBoost classifier
```

MATLAB `fitcauto` selected LogitBoost with minimum leaf size 4 and learning
rate `0.022052`. The evaluated task was **fall versus walking**, not empty
room versus occupied room. The reported confusion matrix contains 16 falls
and 13 walking samples: 14 falls were detected, 2 were missed, and all 13
walking samples were rejected as falls.

## Relevance

This is the newest ESP32-specific paper found in the current search. Its
wavelet stage is a candidate offline experiment for the ARMv7 service, but
not a drop-in firmware algorithm: DWT/scattering and LogitBoost need more
memory and a trained model. The small, controlled dataset means the reported
93.10% accuracy cannot be treated as a general presence/motion result.
