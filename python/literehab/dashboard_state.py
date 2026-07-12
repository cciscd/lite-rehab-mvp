from __future__ import annotations

from dataclasses import dataclass

from .multimodal import MultimodalPrediction, POSE_FEATURE_NAMES
from .pose_features import PoseFeatures
from .synchronization import SynchronizedSample


WARNING_QUALITIES = {"too_fast", "insufficient_range"}
SESSION_FIELDS = (
    "t_ms", "received_s", "ax", "ay", "az", "gx", "gy", "gz",
    "state", "rep_count", "quality", *POSE_FEATURE_NAMES,
    "model_exercise", "model_quality", "model_confidence",
    "visual_confidence", "subject", "label_exercise", "label_quality",
)


@dataclass(frozen=True)
class ResolvedDecision:
    exercise: str
    quality: str
    source: str
    confidence: float


def resolve_decision(
    rule_exercise: str,
    rule_quality: str,
    model: MultimodalPrediction | None,
    threshold: float,
) -> ResolvedDecision:
    if rule_quality in WARNING_QUALITIES:
        return ResolvedDecision(rule_exercise, rule_quality, "rule safety", 1.0)
    if model is not None and model.confidence >= threshold:
        return ResolvedDecision(model.exercise, model.quality,
                                "multimodal model", model.confidence)
    return ResolvedDecision(rule_exercise, rule_quality, "rule fallback",
                            model.confidence if model is not None else 0.0)


def trunk_compensation_active(state: str, pose: PoseFeatures | None,
                              threshold: float = 0.15) -> bool:
    return bool(state != "idle" and pose is not None and pose.valid and
                pose.trunk_displacement >= threshold)


class CameraHealth:
    def __init__(self, max_consecutive_failures: int = 3) -> None:
        if max_consecutive_failures <= 0:
            raise ValueError("max_consecutive_failures must be positive")
        self.max_consecutive_failures = max_consecutive_failures
        self.failures = 0

    def record(self, success: bool) -> bool:
        if success:
            self.failures = 0
        else:
            self.failures += 1
        return self.failures < self.max_consecutive_failures


def synchronized_row(
    synchronized: SynchronizedSample,
    prediction: MultimodalPrediction | None,
    subject: str = "",
    label_exercise: str = "",
    label_quality: str = "",
) -> dict[str, object]:
    sample = synchronized.telemetry
    row: dict[str, object] = {
        "t_ms": sample.timestamp_ms,
        "received_s": synchronized.received_s,
        "ax": sample.accel_g[0],
        "ay": sample.accel_g[1],
        "az": sample.accel_g[2],
        "gx": sample.gyro_dps[0],
        "gy": sample.gyro_dps[1],
        "gz": sample.gyro_dps[2],
        "state": sample.state,
        "rep_count": sample.rep_count,
        "quality": sample.quality,
    }
    pose_values = ((synchronized.pose.to_vector()) if synchronized.pose is not None
                   else (0.0,) * len(POSE_FEATURE_NAMES))
    row.update(dict(zip(POSE_FEATURE_NAMES, pose_values)))
    row.update({
        "model_exercise": prediction.exercise if prediction else "",
        "model_quality": prediction.quality if prediction else "",
        "model_confidence": prediction.confidence if prediction else 0.0,
        "visual_confidence": prediction.visual_confidence if prediction else 0.0,
        "subject": subject,
        "label_exercise": label_exercise,
        "label_quality": label_quality,
    })
    return row
