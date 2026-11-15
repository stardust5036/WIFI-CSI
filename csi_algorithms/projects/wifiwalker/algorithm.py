"""Pure-Python approximation of the documented filter/features boundary."""
from math import sqrt


def magnitude(iq):
    return [sqrt(float(real) ** 2 + float(imag) ** 2) for real, imag in iq]


def zero_crossing_rate(values, threshold=0.0):
    if not values:
        return 0.0
    crossings = sum((left > threshold) != (right > threshold)
                    for left, right in zip(values, values[1:]))
    return crossings / len(values)
