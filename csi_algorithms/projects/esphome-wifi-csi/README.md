# esphome-wifi-csi

- **Repository:** [PeterkoCZ91/esphome-wifi-csi](https://github.com/PeterkoCZ91/esphome-wifi-csi)
- **Found by:** GitHub repository search, updated November 2026

## Scope

An ESPHome external component derived from ESPectre, targeting presence,
motion, breathing-rate detection, pairwise sensing, multi-node fusion, MQTT,
and Home Assistant.

## Algorithm verified from repository search

The repository exposes two detector paths:

```text
CSI IQ
  -> spatial turbulence = standard deviation of selected subcarrier amplitudes
  -> optional 7-sample Hampel/MAD outlier removal
  -> optional low-pass filter
  -> moving variance of turbulence (MVS)
  -> adaptive threshold + hysteresis
```

The code search also identifies an ML path:

```text
turbulence buffer
  -> 17 features including phase/ratio/breathing/DSER/PLCR
  -> MLP 17 -> 18 -> 9 -> 1
  -> probability threshold
```

The repository documents default Hampel window `7`, threshold `5.0 MAD`, MVS
threshold `1.0`, and ML threshold `0.5`; its runtime configuration supports
other values. Its documentation warns that the shipped ML weights were
trained at a different logging cadence from the firmware feature window, so
the default MVS detector is preferred.

## Relevance

This is a deployment-oriented alternative to the current custom firmware.
Its pairwise and multi-node design is worth comparing with the per-peer
diagnostic packets, but its thresholds and processing must be verified from
the selected commit before reuse.
