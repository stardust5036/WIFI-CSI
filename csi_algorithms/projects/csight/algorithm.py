"""Dependency-free reproduction of CSIght's motion and EMA vital stages."""
from math import sqrt


class Detector:
    def __init__(self, baseline_frames=30, sensitivity=5):
        self.baseline_frames = baseline_frames
        self.threshold = 8.0 - sensitivity * 0.7
        self.baseline, self.ready = [], False

    def update(self, iq):
        amplitudes = [sqrt(float(real) ** 2 + float(imag) ** 2)
                      for real, imag in iq]
        if not self.ready:
            self.baseline.append(amplitudes)
            if len(self.baseline) >= self.baseline_frames:
                self.baseline = [sum(row[i] for row in self.baseline) /
                                 len(self.baseline) for i in range(len(amplitudes))]
                self.ready = True
            return False, 0.0
        deltas = [abs(value - base) for value, base in zip(amplitudes, self.baseline)]
        score = sum(deltas) / len(deltas) if deltas else 0.0
        self.baseline = [0.98 * base + 0.02 * value
                         for base, value in zip(self.baseline, amplitudes)]
        return score > self.threshold, score
