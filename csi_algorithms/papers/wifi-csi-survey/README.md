# WiFi Sensing with Channel State Information: A Survey

- **Type:** survey
- **Source:** [William & Mary author version](https://www.cs.wm.edu/~yma/files/WiFiSensing_YongsenMa_authorversion.pdf)
- **Use here:** taxonomy and signal-processing background

## Algorithm findings

CSI is represented as a complex response per subcarrier:

```text
H_k(t) = I_k(t) + j Q_k(t)
amplitude = |H_k(t)|
phase = atan2(Q_k(t), I_k(t))
```

The survey separates pattern-based methods from model-based methods. A
practical pattern-based presence/motion pipeline is amplitude/phase
preprocessing, temporal feature extraction, and thresholding or classification.
Model-based methods use propagation geometry or Fresnel-zone reasoning and
need stronger assumptions about the deployment.

## Relevance and limitations

The survey supports using amplitude, phase, variance, correlation, PCA,
time-frequency features, and supervised classifiers as distinct method
families. It does not provide universal thresholds for an ESP32 deployment;
radio chipset, placement, bandwidth, sampling cadence, and room geometry
change the distributions.
