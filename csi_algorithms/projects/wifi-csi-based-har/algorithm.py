"""Reference parser/calibration stages from WIFI_CSI_based_HAR."""
import struct
from math import atan2, pi


def decode_iq8(payload):
    values = struct.unpack("%db" % len(payload), payload)
    return list(zip(values[0::2], values[1::2]))


def calibrate_phase(phases):
    if not phases:
        return []
    unwrapped, turns = [phases[0]], 0
    for previous, current in zip(phases, phases[1:]):
        if abs(current - previous) > pi:
            turns += 1 if current > previous else -1
        unwrapped.append(current - turns * 2 * pi)
    slope = (unwrapped[-1] - unwrapped[0]) / max(1, len(unwrapped) - 1)
    center = sum(unwrapped) / len(unwrapped)
    return [value - slope * index - center for index, value in enumerate(unwrapped)]


def complex_phase(iq):
    return [atan2(imag, real) for real, imag in iq]
