"""Benchmark preprocessing adapter; model training remains dataset-specific."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, mean, variance


def feature_vector(frame):
    values = amplitudes(frame)
    return [mean(values), variance(values) ** 0.5, min(values), max(values)]
