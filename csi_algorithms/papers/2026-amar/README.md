# AMAR: Lightweight Attention-Based Multi-User Activity Recognition

- **Published:** 20 May 2026
- **Source:** [arXiv:2605.20649](https://arxiv.org/abs/2605.20649)

## Algorithm

AMAR treats simultaneous activities as an unordered set instead of assigning
fixed user identities. Its architecture is:

```text
CSI tensor
  -> lightweight edge CNN feature extractor
  -> residual vector quantization (RVQ)
  -> compressed tokens over the link
  -> cloud Transformer
  -> query-to-CSI cross-attention
  -> query self-attention for activity interaction
  -> set-matched activity outputs and occupancy count
```

Learnable query embeddings act as activity detectors. The reported system has
0.32M parameters and claims over 99.2% bandwidth reduction from the edge/cloud
split. It evaluates count-based precision/recall, perfect-prediction score,
and occupancy-counting error rather than only single-label accuracy.

## Relevance

The work is relevant to future multi-person or multi-activity expansion. It
is much heavier than the current per-peer threshold/state-machine design.
For the current system, its most useful lesson is to keep occupancy count and
activity state as separate outputs; do not force several people into one
boolean motion flag.
