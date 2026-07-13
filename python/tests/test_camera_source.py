from __future__ import annotations

import pytest

from literehab.camera_source import (
    CameraSource,
    parse_camera_source,
    probe_local_cameras,
)


class FakeCapture:
    def __init__(self, source, frames, opened=True):
        self.source = source
        self.frames = list(frames)
        self.opened = opened
        self.released = False
        self.properties = {}

    def isOpened(self):
        return self.opened and not self.released

    def read(self):
        return self.frames.pop(0) if self.frames else (False, None)

    def set(self, key, value):
        self.properties[key] = value
        return True

    def release(self):
        self.released = True


class FakeCV2:
    CAP_PROP_BUFFERSIZE = 1
    CAP_PROP_FRAME_WIDTH = 2
    CAP_PROP_FRAME_HEIGHT = 3

    def __init__(self):
        self.plans = []
        self.captures = []

    def VideoCapture(self, source):
        plan = self.plans.pop(0) if self.plans else {"frames": []}
        capture = FakeCapture(
            source,
            plan.get("frames", []),
            plan.get("opened", True),
        )
        self.captures.append(capture)
        return capture


class FakeClock:
    def __init__(self):
        self.now = 0.0

    def __call__(self):
        return self.now

    def advance(self, seconds):
        self.now += seconds


@pytest.fixture
def fake_cv2():
    return FakeCV2()


@pytest.fixture
def clock():
    return FakeClock()


def test_parse_camera_source_accepts_index_auto_and_rtsp():
    assert parse_camera_source(2) == 2
    assert parse_camera_source("2") == 2
    assert parse_camera_source(" auto ") == "auto"
    assert parse_camera_source("rtsp://10.131.167.1:8554/live") == (
        "rtsp://10.131.167.1:8554/live"
    )


@pytest.mark.parametrize("value", ["", "-1", "http://camera/live", "abc"])
def test_parse_camera_source_rejects_invalid_values(value):
    with pytest.raises(ValueError):
        parse_camera_source(value)


def test_probe_returns_only_indices_that_deliver_a_frame(fake_cv2):
    fake_cv2.plans = [
        {"frames": [(False, None)]},
        {"frames": [(True, object())]},
    ]

    assert probe_local_cameras(fake_cv2, indices=range(2)) == [1]
    assert all(capture.released for capture in fake_cv2.captures)


def test_camera_source_opens_uvc_with_low_latency_settings(fake_cv2):
    fake_cv2.plans = [{"frames": [(True, object())]}]

    source = CameraSource(1, fake_cv2)

    assert source.active_source == 1
    capture = fake_cv2.captures[0]
    assert capture.properties[fake_cv2.CAP_PROP_FRAME_WIDTH] == 640
    assert capture.properties[fake_cv2.CAP_PROP_FRAME_HEIGHT] == 480
    assert capture.properties[fake_cv2.CAP_PROP_BUFFERSIZE] == 1


def test_camera_source_accepts_rtsp_url(fake_cv2):
    url = "rtsp://10.131.167.1:8554/live"
    fake_cv2.plans = [{"frames": [(True, object())]}]

    source = CameraSource(url, fake_cv2)

    assert fake_cv2.captures[0].source == url
    assert source.active_source == url


def test_camera_source_becomes_unhealthy_then_rate_limits_reopen(
    fake_cv2, clock
):
    recovered_frame = object()
    fake_cv2.plans = [
        {"frames": [(False, None)] * 3},
        {"frames": [(True, recovered_frame)]},
    ]
    source = CameraSource(
        0,
        fake_cv2,
        reconnect_interval_s=1.0,
        max_consecutive_failures=3,
        clock=clock,
    )

    source.read()
    source.read()
    source.read()
    assert not source.healthy

    source.read()
    assert len(fake_cv2.captures) == 1

    clock.advance(1.0)
    ok, frame = source.read()
    assert ok
    assert frame is recovered_frame
    assert source.healthy
    assert len(fake_cv2.captures) == 2
    assert fake_cv2.captures[0].released


def test_auto_source_retries_when_camera_appears(fake_cv2, clock):
    frame = object()
    fake_cv2.plans = [
        {"opened": False},
        {"opened": False},
        {"frames": [(True, frame)]},
        {"frames": [(True, frame)]},
    ]
    source = CameraSource(
        "auto", fake_cv2, probe_indices=range(2), reconnect_interval_s=1.0,
        clock=clock
    )

    assert not source.healthy
    assert source.read() == (False, None)
    clock.advance(1.0)
    ok, returned = source.read()

    assert ok
    assert returned is frame
    assert source.active_source == 0


def test_close_releases_capture(fake_cv2):
    fake_cv2.plans = [{"frames": []}]
    source = CameraSource(0, fake_cv2)

    source.close()

    assert fake_cv2.captures[0].released
    assert source.status == "closed"

