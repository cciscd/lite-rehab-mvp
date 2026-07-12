import pytest

from literehab.telemetry import parse_telemetry_line


def test_parses_valid_receiver_line():
    sample = parse_telemetry_line(
        "IMU,1234,0.1,-0.2,1.0,10,-20,30,elbow_flexion,4,ok"
    )
    assert sample is not None
    assert sample.timestamp_ms == 1234
    assert sample.accel_g == pytest.approx((0.1, -0.2, 1.0))
    assert sample.gyro_dps == pytest.approx((10.0, -20.0, 30.0))
    assert sample.state == "elbow_flexion"
    assert sample.rep_count == 4


@pytest.mark.parametrize("line", ["", "# connected", "garbage", "IMU,1,2"])
def test_ignores_comments_and_malformed_lines(line):
    assert parse_telemetry_line(line) is None


def test_rejects_unknown_state():
    assert parse_telemetry_line(
        "IMU,1,0,0,1,0,0,0,running,0,none"
    ) is None
