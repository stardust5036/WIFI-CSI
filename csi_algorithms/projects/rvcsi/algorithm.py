"""Reference of rvCSI's validation boundary and window summary."""
from math import sqrt


def validate_frame(iq):
    if not iq or any(len(pair) != 2 for pair in iq):
        return False, 0.0
    finite = all(abs(float(value)) < 1e9 for pair in iq for value in pair)
    return finite, 1.0 if finite else 0.0


def window_score(frames):
    values = [sqrt(float(i) ** 2 + float(q) ** 2)
              for frame in frames for i, q in frame]
    if not values:
        return 0.0
    center = sum(values) / len(values)
    return sum(abs(value - center) for value in values) / len(values)
