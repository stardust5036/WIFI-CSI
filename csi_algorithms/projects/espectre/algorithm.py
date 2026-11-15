"""Spectral reference only; exact ESPectre parameters require a pinned commit."""
import cmath
from math import pi


def spectrum(values):
    n = len(values)
    if not n:
        return []
    return [sum(values[k] * cmath.exp(-2j * pi * frequency * k / n)
                for k in range(n)) for frequency in range(n // 2 + 1)]


def band_energy(values, low, high):
    return sum(abs(value) ** 2 for value in spectrum(values)[low:high])
