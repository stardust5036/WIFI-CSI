"""Acquisition-side decoding helper; the project itself is not a classifier."""
from struct import unpack


def decode_int8_iq(payload):
    values = unpack("%db" % len(payload), payload)
    return list(zip(values[0::2], values[1::2]))


def decode_int16_iq(payload):
    if len(payload) % 4:
        raise ValueError("int16 IQ payload must contain complete I/Q pairs")
    values = unpack("<%dh" % (len(payload) // 2), payload)
    return list(zip(values[0::2], values[1::2]))
