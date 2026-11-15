"""Minimal documented sliding-variance movement stage; fusion is out of scope."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import spatial_turbulence, moving_variance


def detect(frames, window=20, threshold=1.0):
    values = [spatial_turbulence(frame) for frame in frames]
    result = []
    for index in range(len(values)):
        current = moving_variance(values[:index + 1], window)
        result.append(bool(current is not None and current >= threshold))
    return result
