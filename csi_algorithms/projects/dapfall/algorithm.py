"""Small amplitude histogram/profile representation used by DapFall-style code."""


def amplitude_profile(values, bins=16, minimum=0.0, maximum=100.0):
    if bins <= 0 or maximum <= minimum:
        raise ValueError("invalid histogram range")
    counts = [0] * bins
    width = (maximum - minimum) / bins
    for value in values:
        index = int((value - minimum) / width)
        index = max(0, min(bins - 1, index))
        counts[index] += 1
    total = float(len(values) or 1)
    return [count / total for count in counts]
