"""Small covariance/MUSIC input reference; not a full ESPCAL implementation."""
from math import cos, sin


def covariance(samples):
    if not samples:
        return []
    width = len(samples[0])
    center = [sum(row[i] for row in samples) / len(samples) for i in range(width)]
    return [[sum((row[i] - center[i]) * (row[j] - center[j])
                 for row in samples) / len(samples) for j in range(width)]
            for i in range(width)]


def steering(angle, sensors):
    return [complex(cos(index * angle), sin(index * angle))
            for index in range(sensors)]
