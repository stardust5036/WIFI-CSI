# CSIght

- **Repository:** [joelewis012/CSIght](https://github.com/joelewis012/CSIght)
- **Observed popularity:** 182 stars, 18 forks
- **Source checked:** `esp32/components/csi_handler/csi_handler.c`

## Raw CSI -> result

```text
ESP32 CSI callback
  -> int8 I/Q pairs
  -> amplitude sqrt(I^2 + Q^2)
  -> 30-frame per-subcarrier baseline
  -> absolute amplitude delta
  -> average delta across subcarriers
  -> sensitivity threshold
  -> motion/proximity callback
```

The same source also implements a waterfall path and a separate vital-sign
path: three EMA time constants, adjacent-EMA subtraction, and zero-crossing
counts over approximately 30 seconds. This is directly useful for an
embedded comparison, but its sensitivity and frame-rate assumptions are
project-specific.
