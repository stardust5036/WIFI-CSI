# Device-Free WiFi Human Sensing: From Pattern-Based to Model-Based Approaches

- **Type:** survey/tutorial
- **Source:** [IEEE Xplore](https://ieeexplore.ieee.org/abstract/document/8067692)
- **Use here:** distinction between pattern and physical-model approaches

## Algorithm findings

Pattern-based systems learn or threshold signal changes caused by a person.
Model-based systems infer location or motion from a propagation model, such as
Fresnel-zone effects. The pattern-based route is simpler for an embedded
presence/motion detector:

```text
CSI -> clean/normalize -> temporal features -> detector -> temporal smoothing
```

The model-based route can be more interpretable spatially but requires
calibration of geometry and multipath assumptions.

## Relevance and limitations

For the current system, use pattern-based features for the first implementation.
Do not call a threshold detector a physical localization algorithm merely
because it uses CSI.
