# ESPectre

- **Source:** [francescopace/espectre](https://github.com/francescopace/espectre)

## Project scope

ESPectre is a community Wi-Fi CSI spectral-analysis motion project, with
integration-oriented deployment goals such as Home Assistant.

## Algorithm boundary

It is a project-specific spectral implementation, not Espressif's
`esp_radar_motion_dec`. Its filters, windows, spectral bands, and thresholds
must be read from its current source/configuration before porting. Parameters
must not be copied directly to this ESP32-S3 deployment.
