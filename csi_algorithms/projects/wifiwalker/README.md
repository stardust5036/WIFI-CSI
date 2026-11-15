# wifiwalker

- **Repository:** [mobinets/wifiwalker](https://github.com/mobinets/wifiwalker)
- **Observed popularity:** 6 stars, 1 fork
- **Source checked:** MATLAB `GetDWTcomponents.m`, `MyButterworth.m`,
  `MAD.m`, `ZCR.m`

## Raw CSI -> result

```text
Intel CSI trace
  -> select antenna/link dimensions
  -> magnitude
  -> order-10 Butterworth low-pass, cutoff 0.06
  -> PCA, retain components 2..5
  -> 3-level db4 DWT approximation
  -> spectral/statistical features including ZCR
  -> kNN/biometric walking classifier
```

This is a person-identification/walking project, not presence detection, but
its filtering chain is a useful classical CSI example.
