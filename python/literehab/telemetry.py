from __future__ import annotations

from dataclasses import dataclass


VALID_STATES = {"idle", "forearm_rotation", "elbow_flexion"}
VALID_QUALITIES = {"none", "ok", "too_fast", "insufficient_range"}


@dataclass(frozen=True)
class TelemetrySample:
    timestamp_ms: int
    accel_g: tuple[float, float, float]
    gyro_dps: tuple[float, float, float]
    state: str
    rep_count: int
    quality: str


def parse_telemetry_line(line: str) -> TelemetrySample | None:
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    fields = line.split(",")
    if len(fields) != 11 or fields[0] != "IMU":
        return None
    try:
        timestamp = int(fields[1])
        accel = tuple(float(value) for value in fields[2:5])
        gyro = tuple(float(value) for value in fields[5:8])
        state = fields[8]
        reps = int(fields[9])
        quality = fields[10]
    except (TypeError, ValueError):
        return None
    if state not in VALID_STATES or quality not in VALID_QUALITIES:
        return None
    if timestamp < 0 or reps < 0:
        return None
    return TelemetrySample(timestamp, accel, gyro, state, reps, quality)
