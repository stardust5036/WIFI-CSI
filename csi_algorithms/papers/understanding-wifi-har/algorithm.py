"""HAR preprocessing reference: amplitude stream, normalization, fixed windows."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, mean, variance


def normalize_window(frames):
    values = [amplitudes(frame) for frame in frames]
    flat = [item for row in values for item in row]
    center = mean(flat)
    scale = variance(flat) ** 0.5 or 1.0
    return [[(item - center) / scale for item in row] for row in values]
