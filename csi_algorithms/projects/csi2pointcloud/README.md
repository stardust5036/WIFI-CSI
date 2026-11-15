# csi2pointcloud

- **Repository:** [Arritmic/csi2pointcloud](https://github.com/Arritmic/csi2pointcloud)
- **Observed popularity:** 50 stars, 8 forks
- **Source paths checked:** `src/csi2pointcloud`,
  `scripts/train_model.py`, `scripts/test_model.py`

## Raw CSI -> result

```text
CSI sequence
  -> dataset loader and temporal augmentation
  -> spatio-temporal encoder
  -> Transformer-like point-cloud representation
  -> 3D point-cloud loss
  -> reconstructed human point cloud
```

This is a trained 3D reconstruction system, not a lightweight presence
threshold detector. Its implementation file documents the input contract
only; model weights and training dependencies are required for prediction.
