"""Occupancy reference: low-dimensional temporal variance with a threshold."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import spatial_turbulence, moving_variance


def occupancy(frames, window=20, threshold=1.0):
    values = [spatial_turbulence(frame) for frame in frames]
    score = moving_variance(values, window)
    return score is not None and score >= threshold, score
