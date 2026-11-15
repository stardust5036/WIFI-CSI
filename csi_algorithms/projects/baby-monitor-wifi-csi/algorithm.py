"""Reference respiration signal and apnea timeout logic."""


class ApneaDetector:
    def __init__(self, timeout_samples=30, minimum_signal=0.1):
        self.timeout_samples = timeout_samples
        self.minimum_signal = minimum_signal
        self.quiet_samples = 0

    def update(self, filtered_breath_signal):
        if abs(filtered_breath_signal) < self.minimum_signal:
            self.quiet_samples += 1
        else:
            self.quiet_samples = 0
        return self.quiet_samples >= self.timeout_samples
