"""Explainable CSI presence and motion detector.

This is a research/reference implementation, not Espressif's proprietary
motion decoder. It deliberately uses Python's standard library only.
"""

from __future__ import annotations

import cmath
import math
import random
from dataclasses import dataclass
from statistics import median
from typing import Iterable, List, Optional, Sequence, Tuple


def decode_iq(samples: bytes, data_type: int = 0) -> List[complex]:
    """Decode interleaved [imag, real] signed IQ samples.

    data_type 0 is int8 IQ and data_type 1 is little-endian int16 IQ.
    """
    result: List[complex] = []
    if data_type == 0:
        width = 2
        for offset in range(0, len(samples) - width + 1, width):
            imag = int.from_bytes(samples[offset : offset + 1], "little", signed=True)
            real = int.from_bytes(samples[offset + 1 : offset + 2], "little", signed=True)
            result.append(complex(real, imag))
    elif data_type == 1:
        width = 4
        for offset in range(0, len(samples) - width + 1, width):
            imag = int.from_bytes(samples[offset : offset + 2], "little", signed=True)
            real = int.from_bytes(samples[offset + 2 : offset + 4], "little", signed=True)
            result.append(complex(real, imag))
    else:
        raise ValueError("data_type must be 0 (int8) or 1 (int16)")
    return result


def amplitudes(iq: Sequence[complex]) -> List[float]:
    return [abs(value) for value in iq]


def _unwrap(phases: Sequence[float]) -> List[float]:
    if not phases:
        return []
    output = [phases[0]]
    for phase in phases[1:]:
        value = phase
        while value - output[-1] > math.pi:
            value -= 2.0 * math.pi
        while value - output[-1] < -math.pi:
            value += 2.0 * math.pi
        output.append(value)
    return output


def detrended_phases(iq: Sequence[complex]) -> List[float]:
    """Unwrap phase and remove the per-frame linear phase trend."""
    phase = _unwrap([cmath.phase(value) for value in iq])
    if len(phase) < 2:
        return phase
    x_mean = (len(phase) - 1) / 2.0
    y_mean = sum(phase) / len(phase)
    denominator = sum((index - x_mean) ** 2 for index in range(len(phase)))
    slope = (
        sum((index - x_mean) * (value - y_mean) for index, value in enumerate(phase))
        / denominator
        if denominator
        else 0.0
    )
    intercept = y_mean - slope * x_mean
    return [value - (slope * index + intercept) for index, value in enumerate(phase)]


def _rms(values: Iterable[float]) -> float:
    values = list(values)
    return math.sqrt(sum(value * value for value in values) / len(values)) if values else 0.0


def _relative_rms(current: Sequence[float], previous: Sequence[float]) -> float:
    if len(current) != len(previous) or not current:
        return 0.0
    changes = [
        min(abs(now - old) / max(abs(old), 1.0), 1.0)
        for now, old in zip(current, previous)
    ]
    return _rms(changes)


def robust_energy(values: Sequence[float]) -> float:
    """Median amplitude energy, less sensitive to one bad subcarrier."""
    return median(values) if values else 0.0


@dataclass
class DetectorConfig:
    calibration_frames: int = 50
    motion_enter: float = 0.10
    motion_exit: float = 0.045
    presence_enter: float = 0.055
    presence_exit: float = 0.025
    confirm_frames: int = 3
    release_frames: int = 5
    baseline_alpha: float = 0.01
    activity_alpha: float = 0.20


@dataclass
class Detection:
    calibrated: bool
    motion: bool
    presence: bool
    motion_score: float
    presence_score: float
    amplitude_score: float
    phase_score: float


class CsiDetector:
    """Per-peer detector with calibration, hysteresis, and debounce."""

    def __init__(self, config: Optional[DetectorConfig] = None) -> None:
        self.config = config or DetectorConfig()
        self.previous_amplitude: Optional[List[float]] = None
        self.previous_phase: Optional[List[float]] = None
        self.baseline_energy = 0.0
        self.baseline_frames = 0
        self.activity = 0.0
        self.motion = False
        self.presence = False
        self._motion_confirm = 0
        self._motion_release = 0
        self._presence_confirm = 0
        self._presence_release = 0

    @property
    def calibrated(self) -> bool:
        return self.baseline_frames >= self.config.calibration_frames

    def update_iq(self, iq: Sequence[complex]) -> Detection:
        current_amplitude = amplitudes(iq)
        current_phase = detrended_phases(iq)
        if not current_amplitude:
            return Detection(False, self.motion, self.presence, 0.0, 0.0, 0.0, 0.0)

        energy = robust_energy(current_amplitude)
        if self.previous_amplitude is None or len(self.previous_amplitude) != len(current_amplitude):
            self.previous_amplitude = list(current_amplitude)
            self.previous_phase = list(current_phase)
            return Detection(self.calibrated, self.motion, self.presence, 0.0, 0.0, 0.0, 0.0)

        amplitude_score = _relative_rms(current_amplitude, self.previous_amplitude)
        phase_score = _relative_rms(current_phase, self.previous_phase or current_phase)
        self.previous_amplitude = list(current_amplitude)
        self.previous_phase = list(current_phase)

        if not self.calibrated:
            self.baseline_energy += energy
            self.baseline_frames += 1
            if self.calibrated:
                self.baseline_energy /= self.config.calibration_frames
            return Detection(False, self.motion, self.presence, 0.0, 0.0, amplitude_score, phase_score)

        deviation = abs(energy - self.baseline_energy) / max(self.baseline_energy, 1.0)
        presence_score = min(deviation + 0.5 * self.activity, 1.0)
        instantaneous_motion = min(amplitude_score + 0.35 * phase_score, 1.0)
        self.activity = (
            (1.0 - self.config.activity_alpha) * self.activity
            + self.config.activity_alpha * instantaneous_motion
        )
        motion_score = self.activity
        self.baseline_energy = (
            (1.0 - self.config.baseline_alpha) * self.baseline_energy
            + self.config.baseline_alpha * energy
        )

        self._update_state(
            motion_score >= self.config.motion_enter,
            motion_score <= self.config.motion_exit,
            presence_score >= self.config.presence_enter,
            presence_score <= self.config.presence_exit,
        )
        return Detection(True, self.motion, self.presence, motion_score, presence_score,
                         amplitude_score, phase_score)

    def _update_state(
        self,
        motion_entered: bool,
        motion_released: bool,
        presence_entered: bool,
        presence_released: bool,
    ) -> None:
        if not self.motion:
            self._motion_confirm = self._motion_confirm + 1 if motion_entered else 0
            if self._motion_confirm >= self.config.confirm_frames:
                self.motion = True
                self._motion_confirm = 0
        else:
            self._motion_release = self._motion_release + 1 if motion_released else 0
            if self._motion_release >= self.config.release_frames:
                self.motion = False
                self._motion_release = 0

        if not self.presence:
            self._presence_confirm = self._presence_confirm + 1 if presence_entered else 0
            if self._presence_confirm >= self.config.confirm_frames:
                self.presence = True
                self._presence_confirm = 0
        else:
            self._presence_release = self._presence_release + 1 if presence_released else 0
            if self._presence_release >= self.config.release_frames:
                self.presence = False
                self._presence_release = 0


def _demo() -> None:
    random.seed(7)
    detector = CsiDetector()
    for frame in range(140):
        movement = 0.0 if frame < 70 else 18.0 * math.sin(frame * 0.45)
        iq = [
            complex(
                40.0 + random.uniform(-1.0, 1.0) + movement * math.sin(index * 0.4),
                15.0 + random.uniform(-1.0, 1.0),
            )
            for index in range(32)
        ]
        result = detector.update_iq(iq)
        if frame == 0 or frame == 70 or (frame > 0 and result.motion):
            print(frame, result)


if __name__ == "__main__":
    _demo()
