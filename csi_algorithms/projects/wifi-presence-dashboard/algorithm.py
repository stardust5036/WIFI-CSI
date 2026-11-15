"""Small baseline/deviation reference for the dashboard project."""
from math import sqrt


class Presence:
    def __init__(self, baseline_frames=50, threshold=1.0):
        self.baseline_frames, self.threshold = baseline_frames, threshold
        self.samples, self.reference = [], None

    def update(self, amplitudes):
        if self.reference is None:
            self.samples.append(list(amplitudes))
            if len(self.samples) >= self.baseline_frames:
                self.reference = [sum(row[i] for row in self.samples) /
                                  len(self.samples) for i in range(len(amplitudes))]
            return False, 0.0
        score = sqrt(sum((a - b) ** 2 for a, b in zip(amplitudes, self.reference)) /
                     max(1, len(amplitudes)))
        return score >= self.threshold, score
