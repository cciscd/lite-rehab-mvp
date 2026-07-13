from __future__ import annotations

import time
from collections.abc import Callable, Iterable
from typing import Any


CameraInput = int | str


def parse_camera_source(value: CameraInput) -> CameraInput:
    """Normalize a local camera index, ``auto``, or an RTSP URL."""
    if isinstance(value, bool):
        raise ValueError(
            "camera source must be auto, a non-negative index, or rtsp:// URL"
        )
    if isinstance(value, int):
        if value < 0:
            raise ValueError("camera index must be non-negative")
        return value

    text = str(value).strip()
    if text.lower() == "auto":
        return "auto"
    if text.isdigit():
        return int(text)
    if text.lower().startswith("rtsp://") and len(text) > len("rtsp://"):
        return text
    raise ValueError(
        "camera source must be auto, a non-negative index, or rtsp:// URL"
    )


def probe_local_cameras(
    cv2_module: Any,
    indices: Iterable[int] = range(5),
) -> list[int]:
    """Return local camera indices that open and deliver one complete frame."""
    available: list[int] = []
    for index in indices:
        capture = cv2_module.VideoCapture(index)
        try:
            ok, frame = capture.read() if capture.isOpened() else (False, None)
            if ok and frame is not None:
                available.append(index)
        finally:
            capture.release()
    return available


class CameraSource:
    """OpenCV camera input with bounded probing and rate-limited recovery."""

    def __init__(
        self,
        source: CameraInput,
        cv2_module: Any,
        reconnect_interval_s: float = 1.0,
        max_consecutive_failures: int = 3,
        probe_indices: Iterable[int] = range(5),
        clock: Callable[[], float] = time.monotonic,
    ) -> None:
        if reconnect_interval_s <= 0:
            raise ValueError("reconnect_interval_s must be positive")
        if max_consecutive_failures <= 0:
            raise ValueError("max_consecutive_failures must be positive")

        self.requested_source = parse_camera_source(source)
        self.cv2 = cv2_module
        self.reconnect_interval_s = reconnect_interval_s
        self.max_consecutive_failures = max_consecutive_failures
        self.probe_indices = tuple(probe_indices)
        self.clock = clock
        self.capture = None
        self.active_source: CameraInput | None = None
        self.failures = 0
        self.next_reconnect_s = 0.0
        self.status = "not connected"
        self._open()

    @property
    def healthy(self) -> bool:
        return bool(
            self.capture is not None
            and self.capture.isOpened()
            and self.failures < self.max_consecutive_failures
        )

    def _resolve(self) -> CameraInput | None:
        if self.requested_source != "auto":
            return self.requested_source
        for index in self.probe_indices:
            capture = self.cv2.VideoCapture(index)
            try:
                ok, frame = (
                    capture.read() if capture.isOpened() else (False, None)
                )
                if ok and frame is not None:
                    return index
            finally:
                capture.release()
        return None

    def _set_capture_properties(self, capture: Any) -> None:
        capture.set(self.cv2.CAP_PROP_BUFFERSIZE, 1)
        capture.set(self.cv2.CAP_PROP_FRAME_WIDTH, 640)
        capture.set(self.cv2.CAP_PROP_FRAME_HEIGHT, 480)

    def _open(self) -> None:
        selected = self._resolve()
        if selected is None:
            self.status = "no local camera found; retrying"
            self.next_reconnect_s = self.clock() + self.reconnect_interval_s
            return

        capture = self.cv2.VideoCapture(selected)
        self._set_capture_properties(capture)
        if capture.isOpened():
            self.capture = capture
            self.active_source = selected
            self.failures = 0
            self.status = f"connected: {selected}"
            return

        capture.release()
        self.capture = None
        self.active_source = None
        self.status = f"unavailable: {selected}; retrying"
        self.next_reconnect_s = self.clock() + self.reconnect_interval_s

    def read(self) -> tuple[bool, Any | None]:
        if self.capture is None or self.failures >= self.max_consecutive_failures:
            if self.clock() < self.next_reconnect_s:
                return False, None
            self._release()
            self._open()

        if self.capture is None:
            return False, None

        ok, frame = self.capture.read()
        if ok and frame is not None:
            self.failures = 0
            self.status = f"connected: {self.active_source}"
            return True, frame

        self.failures += 1
        self.status = (
            f"frame failure {self.failures}/{self.max_consecutive_failures}"
        )
        if self.failures >= self.max_consecutive_failures:
            self.next_reconnect_s = self.clock() + self.reconnect_interval_s
            self.status += "; retrying"
        return False, None

    def _release(self) -> None:
        if self.capture is not None:
            self.capture.release()
        self.capture = None
        self.active_source = None

    def close(self) -> None:
        self._release()
        self.status = "closed"
