from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class FusionResult:
    mode: str
    feedback: str


def fuse_feedback(
    state: str,
    imu_quality: str,
    elbow_range_deg: float | None,
    trunk_flag: bool | None,
    camera_available: bool,
) -> FusionResult:
    mode = "Fusion" if camera_available else "IMU-only"
    if imu_quality == "too_fast":
        return FusionResult(mode, "Move more slowly")
    if imu_quality == "insufficient_range":
        return FusionResult(mode, "Increase movement range")
    if imu_quality == "trunk_compensation":
        return FusionResult(mode, "Avoid trunk compensation")
    if camera_available and state != "idle" and trunk_flag:
        return FusionResult(mode, "Avoid trunk compensation")
    if (
        camera_available
        and state == "elbow_flexion"
        and elbow_range_deg is not None
        and elbow_range_deg < 35.0
    ):
        return FusionResult(mode, "Increase movement range")
    if imu_quality == "ok":
        return FusionResult(mode, "Good repetition")
    return FusionResult(mode, "Ready")
