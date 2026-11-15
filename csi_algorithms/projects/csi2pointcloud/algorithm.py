"""Input contract for the CSI-to-point-cloud model."""


def make_sequence(frames, sequence_length):
    if sequence_length <= 0:
        raise ValueError("sequence_length must be positive")
    return [frames[index:index + sequence_length]
            for index in range(len(frames) - sequence_length + 1)]
