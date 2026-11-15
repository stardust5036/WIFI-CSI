"""Dataset window helper; model inference remains repository-specific."""


def windows(samples, size, stride=1):
    if size <= 0 or stride <= 0:
        raise ValueError("size and stride must be positive")
    return [samples[index:index + size]
            for index in range(0, len(samples) - size + 1, stride)]
