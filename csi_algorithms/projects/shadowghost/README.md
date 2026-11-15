# SHADOWGHOST

- **Repository:** [vincenzo-afk/SHADOWGHOST](https://github.com/vincenzo-afk/SHADOWGHOST)
- **Observed popularity:** 36 stars
- **Source checked:** `backend/detector.py`

## Raw CSI -> result

```text
CSI frame
  -> subtract empty-room baseline
  -> reject low-variance frames
  -> StandardScaler
  -> Isolation Forest anomaly score
  -> 56 -> 32 -> 16 -> 32 -> 56 autoencoder reconstruction score
  -> 70% IF + 30% AE
  -> anomaly threshold
  -> zone energy and center-of-mass
  -> movement / occupancy / zone result
```

This is one of the more concrete open-source anomaly-detection projects
found in the second search pass.
