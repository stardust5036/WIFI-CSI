"""Time-selective RNN input preparation; recurrent weights are not included."""


def select_periodic(frames, stride):
    if stride <= 0:
        raise ValueError("stride must be positive")
    return frames[::stride]


def select_random(frames, indices):
    return [frames[index] for index in indices if 0 <= index < len(frames)]
