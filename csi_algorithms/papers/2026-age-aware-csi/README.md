# Resource-Efficient Wi-Fi CSI-Based Sensing via the Age of Samples

- **Published:** 30 June 2026
- **Source:** [arXiv:2606.31690](https://arxiv.org/abs/2606.31690)

## Algorithm

The method models a sensing session as opportunities indexed by `t`. It
evaluates deterministic accumulated sampling and Bernoulli sampling. The
retained frames are packed with an age value
`delta = decision_index - sample_index`.

The model has two branches:

```text
packed CSI [antennas, subcarriers, retained frames]
  -> three convolution blocks
  -> adaptive pooling
  -> 128-dimensional CSI embedding

sample ages
  -> normalized age
  -> linear + batch normalization + sigmoid
  -> average age embedding

element-wise CSI embedding * age embedding
  -> small post-fusion block
  -> classifier
```

Only amplitude is used because commodity phase is strongly affected by CFO,
SFO, and packet-detection delay. The age branch is a freshness gate, not a
replacement for CSI features.

## Relevance

The current service should preserve sequence timestamps and sequence numbers.
If UDP loss or deliberate sampling is added later, the detector should not
pretend that an irregular sequence is uniformly sampled. The simplest
portable adaptation is to include inter-frame age in the motion smoothing
coefficient or to reject stale samples.
