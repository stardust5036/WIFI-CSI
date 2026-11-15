"""Commodity-IoT motion baseline: temporal amplitude difference and RMS."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import amplitudes, rms


def motion_score(previous, current):
    before, after = amplitudes(previous), amplitudes(current)
    if len(before) != len(after):
        raise ValueError("frames must have equal subcarrier counts")
    return rms([a - b for a, b in zip(after, before)])
