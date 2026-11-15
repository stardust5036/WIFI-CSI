# WiFi-3D-Fusion

- **Repository:** [MaliosDark/wifi-3d-fusion](https://github.com/MaliosDark/wifi-3d-fusion)
- **Found by:** GitHub repository search, checked November 2026

## Scope

The project combines ESP32 CSI UDP or Nexmon capture with real-time motion
detection, fusion, and 3D visualization. Its README describes sliding
variance/threshold movement detection and optional neural/3D processing.

## Algorithm verified from repository README/code search

The high-level path is:

```text
ESP32 UDP CSI or Nexmon pcap
  -> amplitude/phase extraction
  -> timestamped vector
  -> sliding variance / threshold movement events
  -> fusion of CSI/RSSI evidence
  -> optional CNN, pose/re-identification, and 3D viewer
```

The repository is a broad research/visualization monorepo. The sliding
variance detector is the directly comparable part; the CNN, 3D pose,
continuous-learning, and visualization stages are separate heavier layers.
Its README also documents ESP32 UDP defaults that differ from this repository,
so those values must not be copied into the current UDP port/protocol.

## Relevance

The simple movement stage is relevant to the ARMv7 service. The 3D pose,
PyTorch, CUDA, and visualization layers are outside the current minimal
service and should not be confused with the binary UDP parser.
