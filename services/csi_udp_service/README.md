# CSI UDP service

This service runs on ARMv7 without external Rust crates. It receives the
packets broadcast by `wifi_sensing_demo` on UDP port `5005`.

It prints both:

- `fw_motion` / `fw_presence`: decisions reported by the ESP firmware;
- `service_motion` / `service_presence`: an independent result calculated
  from the received CSI IQ samples.

Build for a native ARMv7 Linux host:

```bash
cargo build --release
```

For cross compilation, install an ARMv7 target such as
`armv7-unknown-linux-gnueabihf` and use the matching linker configuration.

Run:

```bash
./target/release/csi_udp_service --bind 0.0.0.0 --port 5005
```

The service uses only the Rust standard library.
