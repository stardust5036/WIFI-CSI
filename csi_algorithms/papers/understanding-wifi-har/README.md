# Understanding and Modeling of WiFi Signal Based Human Activity Recognition

- **Type:** activity-recognition paper
- **Source:** [ACM DOI](https://dl.acm.org/doi/abs/10.1145/2789168.2790093)
- **Use here:** temporal CSI features and activity modeling

## Algorithm findings

The central idea is to model how a time sequence changes under different
human activities rather than classify a single CSI sample. This supports
windowed features such as amplitude variance, first differences, temporal
correlation, and frequency content.

## Relevance and limitations

This is more appropriate for classifying walking/waving/sitting than for a
minimal binary motion flag. A binary system can use the same windowing idea
without requiring a large labeled activity dataset.
