# Ghostline: A Six-Node Wi-Fi CSI Motion Tracker

- **Published:** 13 October 2026
- **Source:** [arXiv:2610.18764](https://arxiv.org/abs/2610.18764)
- **Data/scripts:** [Zenodo DOI 10.5281/zenodo.23337813](https://doi.org/10.5281/zenodo.23337813)

## Algorithm

Ghostline uses six ESP32-S3 boards plus a router, producing up to 36 directed
links over 21 radio spans. The detailed processing chain is:

```text
int8 IQ per link
  -> log(1 + amplitude)
  -> discard DC/guard/always-zero subcarriers
  -> subtract mean per LTF block
  -> six-frame temporal standard deviation per subcarrier
  -> average across subcarriers
  -> 10th-percentile quiet baseline over the last 60 s
  -> ratio to baseline and exponential disturbance score
  -> quality-weighted span evidence
  -> Gaussian link zones on a 0.25 m floor grid
  -> peak extraction and subtraction
  -> constant-velocity Kalman tracks + Hungarian association
```

Important parameters in the scored session include a 0.6--2.0 s feature
window, a 3x baseline outlier guard, disturbance mapping
`1 - exp(-(ratio - 1.4))`, 0.7 m grid-zone width, 0.5 EMA smoothing, and
3-consecutive associations to confirm a track. A silent link is removed after
3 s. The system uses no trained classifier for the tracker.

## Relevance

The important result for this project is architectural: multiple independent
CSI links can improve geometric motion tracking, but link fusion is not
automatically better for binary detection. The pilot reports that a standing
person can disappear within seconds, false tracks occurred in empty periods,
and two walkers were not separated. This must not be presented as reliable
static occupancy detection.
