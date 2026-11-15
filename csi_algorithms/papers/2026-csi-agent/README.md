# CSI-Agent: LLM-Assisted Few-Shot Adaptation

- **Published:** 25 September 2026
- **Source:** [arXiv:2609.31990](https://arxiv.org/abs/2609.31990)

## Algorithm

CSI-Agent treats cross-domain adaptation as deployment-time decision making.
It does not feed raw CSI to the LLM. Instead:

```text
raw CSI
  -> deterministic sensing tools
  -> class-wise activation/error statistics
  -> complementary-view evidence
  -> LLM planner chooses class-specific adaptation action
  -> reliability/no-harm verifier
  -> execute or keep safer fallback
```

The paper reports that a one-shot prototypical network can improve one target
domain while degrading another, and that high source-model confidence is not
reliable under domain shift. The agent therefore makes decisions per class
and validates each intervention.

## Relevance

The useful lesson is that cross-room and cross-device calibration is a core
problem. An LLM agent is not needed in the current embedded data path; the
same concern is handled here by per-peer calibration, logged diagnostics,
explicit thresholds, and a safe fallback when evidence is weak.
