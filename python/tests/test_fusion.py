from literehab.fusion import fuse_feedback


def test_camera_missing_preserves_imu_feedback():
    result = fuse_feedback("elbow_flexion", "ok", None, None, False)
    assert result.mode == "IMU-only"
    assert result.feedback == "Good repetition"


def test_trunk_compensation_has_priority():
    result = fuse_feedback("elbow_flexion", "ok", 95.0, True, True)
    assert result.feedback == "Avoid trunk compensation"


def test_small_elbow_range_is_reported():
    result = fuse_feedback("elbow_flexion", "ok", 22.0, False, True)
    assert result.feedback == "Increase movement range"


def test_imu_warning_is_preserved():
    result = fuse_feedback("forearm_rotation", "too_fast", None, False, True)
    assert result.feedback == "Move more slowly"
