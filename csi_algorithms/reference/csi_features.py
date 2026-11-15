"""Small dependency-free CSI primitives shared by per-item references."""

from math import sqrt


def amplitudes(iq):
    return [sqrt(float(i) * i + float(q) * q) for i, q in iq]


def mean(values):
    return sum(values) / len(values) if values else 0.0


def variance(values):
    if not values:
        return 0.0
    center = mean(values)
    return mean([(value - center) ** 2 for value in values])


def rms(values):
    return sqrt(mean([value * value for value in values]))


def spatial_turbulence(iq):
    return sqrt(variance(amplitudes(iq)))


def moving_variance(values, window):
    if len(values) < window:
        return None
    return variance(values[-window:])


def p95(values):
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, int(0.95 * (len(ordered) - 1)))
    return ordered[index]


def sigmoid(value):
    if value < -60.0:
        return 0.0
    if value > 60.0:
        return 1.0
    from math import exp
    return 1.0 / (1.0 + exp(-value))
