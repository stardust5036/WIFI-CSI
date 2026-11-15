"""Public FSM approximation; the closed esp_radar_motion_dec is unavailable."""


class Hysteresis:
    def __init__(self, enter, exit, confirmations=3):
        self.enter, self.exit = enter, exit
        self.confirmations, self.state, self.count = confirmations, False, 0

    def update(self, jitter):
        limit = self.enter if not self.state else self.exit
        candidate = jitter >= limit if not self.state else jitter > limit
        self.count = self.count + 1 if candidate else 0
        if self.count >= self.confirmations:
            self.state = candidate
            self.count = 0
        return self.state
