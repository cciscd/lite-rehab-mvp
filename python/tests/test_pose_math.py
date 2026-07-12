import pytest

from literehab.pose_math import angle_degrees, trunk_compensation


def test_angle_degrees_for_right_and_straight_angles():
    assert angle_degrees((1, 0), (0, 0), (0, 1)) == pytest.approx(90.0)
    assert angle_degrees((-1, 0), (0, 0), (1, 0)) == pytest.approx(180.0)


def test_angle_returns_none_for_degenerate_points():
    assert angle_degrees((0, 0), (0, 0), (1, 0)) is None


def test_trunk_compensation_is_normalized_by_torso_length():
    baseline = {"shoulder": (0.5, 0.3), "hip": (0.5, 0.7)}
    small = {"shoulder": (0.52, 0.3), "hip": (0.5, 0.7)}
    large = {"shoulder": (0.62, 0.3), "hip": (0.5, 0.7)}
    assert not trunk_compensation(baseline, small, threshold=0.15)
    assert trunk_compensation(baseline, large, threshold=0.15)
