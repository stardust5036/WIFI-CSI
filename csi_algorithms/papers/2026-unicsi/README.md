# UniCSI: Towards a Universal Wi-Fi CSI Encoder

- **Published:** 7 October 2026
- **Source:** [arXiv:2610.09559](https://arxiv.org/abs/2610.09559)

## Algorithm

UniCSI takes amplitude-only CSI tensors with shape
`time x tx x rx x subcarrier`. Each TX/RX link is processed independently:

```text
per-subcarrier temporal 1-D CNN
  -> non-overlapping temporal patches
  -> one token per subcarrier and time patch
  -> fractional physical-frequency positional encoding
  -> learnable query tokens cross-attend over subcarriers
  -> repeated Transformer refinement
  -> fixed spectral signature
  -> Transformer encoder
```

For subcarrier `k`, it uses the centered frequency offset normalized to the
half bandwidth, so subcarriers at equivalent physical positions receive
compatible embeddings even when the number of subcarriers changes. It avoids
zero-padding and subcarrier resampling. Training is either supervised
multi-task learning or JEPA self-supervised prediction with an EMA target
encoder.

## Relevance

This is relevant when combining ESP32 CSI with other radios or bandwidths.
The key portable design idea is to preserve physical subcarrier coordinates
and packet metadata instead of treating array indices as universal. It is a
learned foundation-style encoder, not a direct replacement for the current
no-dependency ARMv7 detector.
