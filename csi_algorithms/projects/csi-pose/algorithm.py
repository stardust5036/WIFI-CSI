"""Pose-output fall rule boundary; neural pose inference is not reimplemented."""


def fall_rule(joints, confidence, minimum_confidence=0.5):
    if confidence < minimum_confidence:
        return False
    required = ("head", "hip", "ankle")
    if not all(name in joints for name in required):
        return False
    head_y = joints["head"][1]
    hip_y = joints["hip"][1]
    ankle_y = joints["ankle"][1]
    return (ankle_y - head_y) < (hip_y - head_y) * 0.75
