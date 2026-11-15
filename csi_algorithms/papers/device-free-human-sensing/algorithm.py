"""Classical device-free baseline: change from an empty-scene reference."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, rms


def deviation(frame, baseline):
    current = amplitudes(frame)
    if len(current) != len(baseline):
        raise ValueError("frame and baseline must have equal subcarrier counts")
    return rms([a - b for a, b in zip(current, baseline)])
