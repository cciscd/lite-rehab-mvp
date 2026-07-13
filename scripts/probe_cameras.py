#!/usr/bin/env python3
"""List local OpenCV cameras that open and deliver one frame."""

from __future__ import annotations

import argparse

import cv2

from literehab.camera_source import probe_local_cameras


def main() -> None:
    parser = argparse.ArgumentParser(description="Find usable local UVC cameras")
    parser.add_argument(
        "--max-index",
        type=int,
        default=4,
        help="highest local camera index to test (default: 4)",
    )
    args = parser.parse_args()
    if args.max_index < 0:
        raise SystemExit("--max-index must be non-negative")

    cameras = probe_local_cameras(cv2, range(args.max_index + 1))
    if not cameras:
        raise SystemExit("No working UVC camera found")

    print("Working local camera indices:", ", ".join(map(str, cameras)))
    print("Tip: disconnect MaixCAM 2, run this once, then reconnect and run again.")
    print("The newly appearing index is normally MaixCAM 2.")
    print("Launch with: ./scripts/start_maixcam2_demo.sh <maixcam-index>")


if __name__ == "__main__":
    main()

