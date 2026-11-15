"""Reference of wifi-ghost's documented P95 calibration and 3/5 hysteresis."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[2] / "reference"))
from csi_features import spatial_turbulence, p95


class Detector:
    def __init__(self, calibration=100, on_hits=3, off_hits=5, multiplier=1.10):
        self.calibration, self.multiplier = calibration, multiplier
        self.on_hits, self.off_hits = on_hits, off_hits
        self.values, self.state, self.hits = [], False, 0

    def update(self, iq):
        value = spatial_turbulence(iq)
        self.values.append(value)
        if len(self.values) <= self.calibration:
            return False, value
        baseline = p95(self.values[:self.calibration]) * self.multiplier
        active = value > baseline
        self.hits = self.hits + 1 if active else self.hits - 1
        self.hits = max(-self.off_hits, min(self.on_hits, self.hits))
        if self.hits >= self.on_hits:
            self.state = True
        if self.hits <= -self.off_hits:
            self.state = False
        return self.state, value
