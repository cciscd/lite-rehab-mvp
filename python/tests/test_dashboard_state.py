from literehab.dashboard_state import (
    CameraHealth,
    resolve_decision,
    synchronized_row,
    trunk_compensation_active,
)
from literehab.multimodal import MultimodalPrediction, POSE_FEATURE_NAMES
from literehab.pose_features import PoseFeatures
from literehab.synchronization import SynchronizedSample
from literehab.telemetry import TelemetrySample


def model_prediction(confidence=0.9):
    return MultimodalPrediction("elbow_flexion", "ok", confidence, 0.8)


def synchronized(pose=None):
    telemetry = TelemetrySample(
        100, (0.1, 0.2, 1.0), (10.0, 20.0, 30.0),
        "forearm_rotation", 2, "none")
    return SynchronizedSample(telemetry, 5.0, pose)


def test_model_requires_confidence_and_never_overrides_rule_warning():
    low = resolve_decision("forearm_rotation", "none", model_prediction(0.4), 0.7)
    high = resolve_decision("forearm_rotation", "none", model_prediction(0.9), 0.7)
    warning = resolve_decision("forearm_rotation", "too_fast",
                               model_prediction(0.9), 0.7)

    assert low.source == "rule fallback"
    assert high.source == "multimodal model"
    assert high.exercise == "elbow_flexion"
    assert warning.quality == "too_fast"
    assert warning.source == "rule safety"


def test_trunk_compensation_only_applies_during_motion():
    pose = PoseFeatures(1.0, True, trunk_displacement=0.25)
    assert not trunk_compensation_active("idle", pose)
    assert trunk_compensation_active("elbow_flexion", pose)


def test_single_camera_failure_does_not_disable_vision_forever():
    health = CameraHealth(max_consecutive_failures=3)
    assert health.record(True)
    assert health.record(False)
    assert health.record(False)
    assert not health.record(False)
    assert health.record(True)


def test_synchronized_row_has_complete_pose_schema_when_vision_missing():
    row = synchronized_row(synchronized(), None, "S01", "elbow_flexion", "ok")

    assert row["subject"] == "S01"
    assert row["label_exercise"] == "elbow_flexion"
    assert row["label_quality"] == "ok"
    assert row["vision_valid"] == 0.0
    assert all(name in row for name in POSE_FEATURE_NAMES)
