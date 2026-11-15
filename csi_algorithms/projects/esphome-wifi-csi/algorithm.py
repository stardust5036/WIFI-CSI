"""Reference of the documented ESPHome detector: turbulence -> MVS -> hysteresis."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import spatial_turbulence, moving_variance


class Detector:
    def __init__(self, window=7, threshold=1.0, on_hits=4, off_hits=5):
        self.window, self.threshold = window, threshold
        self.on_hits, self.off_hits = on_hits, off_hits
        self.values, self.state, self.on_count, self.off_count = [], False, 0, 0

    def update(self, iq):
        self.values.append(spatial_turbulence(iq))
        value = moving_variance(self.values, self.window)
        if value is None:
            return self.state, None
        if value >= self.threshold:
            self.on_count += 1
            self.off_count = 0
        else:
            self.off_count += 1
            self.on_count = 0
        if self.on_count >= self.on_hits:
            self.state = True
        if self.off_count >= self.off_hits:
            self.state = False
        return self.state, value
