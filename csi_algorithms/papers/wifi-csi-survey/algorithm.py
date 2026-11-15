"""Survey baseline: amplitude normalization and temporal variation."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, mean, variance


def extract(frame):
    values = amplitudes(frame)
    center = mean(values)
    return [value - center for value in values], variance(values)
