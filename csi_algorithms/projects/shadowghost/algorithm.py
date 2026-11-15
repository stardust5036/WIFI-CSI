"""Dependency-free scoring skeleton for SHADOWGHOST's feature contract."""
from math import sqrt


def anomaly_score(frame, baseline, threshold=0.5):
    if len(frame) != len(baseline):
        raise ValueError("frame and baseline must have equal lengths")
    delta = [a - b for a, b in zip(frame, baseline)]
    variance = sum(value * value for value in delta) / max(1, len(delta))
    score = min(1.0, sqrt(variance))
    return {"movement": score > threshold, "score": score,
            "variance": variance}
