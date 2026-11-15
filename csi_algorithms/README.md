# CSI Presence and Motion Algorithms

The material is deliberately separated so research evidence is not confused
with an implementation:

```text
csi_algorithms/
  papers/       one directory per paper or survey, with algorithm.py
  projects/     one directory per public implementation, with algorithm.py
  reference/    shared primitives and the current end-to-end baseline
```

Every entry now has an adjacent `algorithm.py`. For classical DSP projects it
implements the documented path. For trained research models it implements the
raw-input/window/feature contract and explicitly leaves learned weights out.
For projects whose checked source is only an API, mock, dataset, or index, the
file raises or exposes that boundary instead of inventing an algorithm.

## Recommended baseline

For the current ESP32-to-ARMv7 pipeline, start with
[algorithm_reference.py](D:/code/CSI/csi_algorithms/reference/algorithm_reference.py):

1. Decode interleaved signed IQ samples.
2. Derive amplitude and detrended phase.
3. Calibrate a per-peer empty-scene baseline.
4. Compute amplitude delta RMS and phase activity.
5. Combine short-term activity with long-term baseline deviation.
6. Apply separate enter/exit thresholds and frame confirmation.

The implementation uses only the Python standard library and supports int8
and int16 CSI. Its thresholds require calibration and are not claimed to be
Espressif's closed `esp_radar_motion_dec` implementation.

See [papers/](D:/code/CSI/csi_algorithms/papers/) for literature and
[projects/](D:/code/CSI/csi_algorithms/projects/) for project-specific facts.
