# csi-pose

- **Repository:** [sel00000/csi-pose](https://github.com/sel00000/csi-pose)
- **Observed popularity:** 15 stars, 2 forks
- **Source checked:** README, firmware CSI link, phase-sanitization figures

## Raw CSI -> result

```text
3 TX x 3 RX ESP32-S3 CSI
  -> amplitude/phase extraction
  -> phase sanitization and temporal representation
  -> learned 2D 18-joint pose estimator
  -> pose coordinates/confidence
  -> rule-based fall detector
```

The repository explicitly separates learned pose inference from the
rule-based fall layer. It is not a lightweight raw amplitude threshold.
