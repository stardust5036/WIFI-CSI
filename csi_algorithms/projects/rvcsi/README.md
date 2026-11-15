# rvCSI

- **Repository:** [ruvnet/rvcsi](https://github.com/ruvnet/rvcsi)
- **Observed popularity:** 30 stars, 6 forks
- **Source paths checked:** `crates/rvcsi-core/src`,
  `crates/rvcsi-adapter-esp32`, `crates/rvcsi-adapter-nexmon`

## Raw CSI -> result

```text
ESP32/Nexmon/file adapter
  -> normalized CsiFrame
  -> frame validation and quality score
  -> bounded CsiWindow
  -> mean amplitude / phase variance / motion / presence scores
  -> typed CsiEvent with confidence and evidence window
```

rvCSI is primarily a Rust normalization/runtime architecture. Its important
algorithmic contribution is the validation boundary and typed event contract;
it does not prove one fixed motion classifier.
