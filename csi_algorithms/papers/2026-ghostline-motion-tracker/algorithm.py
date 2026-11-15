"""Ghostline link-level disturbance and quality-weighted motion score."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import spatial_turbulence, p95


def link_score(frames, baseline_window=60):
    values = [spatial_turbulence(frame) for frame in frames]
    baseline = p95(values[-baseline_window:]) if values else 0.0
    return (values[-1] / baseline) if values and baseline else 0.0
