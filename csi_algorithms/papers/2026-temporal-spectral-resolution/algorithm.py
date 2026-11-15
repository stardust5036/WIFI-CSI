"""Temporal/spectral trade-off reference: block-mean subcarrier grouping."""


def block_mean(values, block_size):
    if block_size <= 0:
        raise ValueError("block_size must be positive")
    return [sum(values[index:index + block_size]) / len(values[index:index + block_size])
            for index in range(0, len(values), block_size)
            if values[index:index + block_size]]
