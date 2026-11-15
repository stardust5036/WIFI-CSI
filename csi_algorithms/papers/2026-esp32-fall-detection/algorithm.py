"""Paper-level fall pipeline: low-pass/DWT placeholder plus fixed-window features."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, mean, variance


def window_features(frames):
    values = [mean(amplitudes(frame)) for frame in frames]
    return {"mean": mean(values), "std": variance(values) ** 0.5,
            "range": max(values) - min(values) if values else 0.0}
