"""UniCSI input adapter: preserve physical frequency coordinates."""


def make_tokens(frames, frequencies):
    if not frames:
        return []
    if any(len(frame) != len(frequencies) for frame in frames):
        raise ValueError("frequency coordinates must match every frame")
    return [[{"frequency": frequency, "amplitude": abs(value)}
             for value, frequency in zip(frame, frequencies)] for frame in frames]
