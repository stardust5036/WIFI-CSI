"""Framework adapter: keep feature extraction and event state separate."""


class EventState:
    def __init__(self, threshold=0.5):
        self.threshold, self.active = threshold, False

    def update(self, confidence):
        self.active = confidence >= self.threshold
        return {"active": self.active, "confidence": confidence}
