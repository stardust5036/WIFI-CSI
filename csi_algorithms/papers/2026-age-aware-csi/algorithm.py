"""Age-aware input: pair each CSI frame with freshness measured in seconds."""


def add_age(frames, timestamps, now):
    if len(frames) != len(timestamps):
        raise ValueError("frames and timestamps must have equal lengths")
    return [{"frame": frame, "age": max(0.0, now - timestamp)}
            for frame, timestamp in zip(frames, timestamps)]
