"""Explicit boundary: no public raw-CSI algorithm was verified."""


def unavailable():
    raise NotImplementedError(
        "The checked repository exposes API/mock pose behavior, not raw CSI DSP"
    )
