# Wisense

- **Repository:** [collabray/wisense](https://github.com/collabray/wisense)
- **Observed popularity:** 33 stars, 5 forks
- **Source paths checked:** `wisense/presence/detector.py`,
  `wisense/fall/detector.py`, `examples/presence_demo.py`

## Algorithm boundary

This is a Python sensing framework with presence/fall detector interfaces and
model registry. The public API separates raw feature ingestion, detector
state, confidence, and event output. The exact model is selected by registry
configuration, so this entry does not invent a single raw-CSI formula.
