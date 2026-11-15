"""Five-second window reference for WiTrace-style occupancy detection."""
from math import sqrt


def window_score(frames, baseline):
    deltas = []
    for frame in frames:
        if len(frame) != len(baseline):
            raise ValueError("CSI frame size changed")
        deltas.extend(abs(value - base) for value, base in zip(frame, baseline))
    if not deltas:
        return 0.0
    center = sum(deltas) / len(deltas)
    return sqrt(sum((value - center) ** 2 for value in deltas) / len(deltas))
