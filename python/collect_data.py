#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import time
from pathlib import Path

import serial
from serial.tools import list_ports

from literehab.telemetry import parse_telemetry_line


def choose_port(requested: str) -> str:
    if requested != "auto":
        return requested
    candidates = [p.device for p in list_ports.comports()]
    for name in candidates:
        if "usbmodem" in name.lower() or "usbserial" in name.lower():
            return name
    if not candidates:
        raise RuntimeError("No serial port found. Connect the ESP32-S3 receiver.")
    return candidates[0]


def main() -> None:
    parser = argparse.ArgumentParser(description="Record labeled LiteRehab IMU data")
    parser.add_argument("--port", default="auto")
    parser.add_argument("--label", required=True,
                        choices=["idle", "forearm_rotation", "elbow_flexion"])
    parser.add_argument("--subject", required=True)
    parser.add_argument("--seconds", type=float, default=30.0)
    parser.add_argument("--output", type=Path, default=Path("data"))
    args = parser.parse_args()

    args.output.mkdir(parents=True, exist_ok=True)
    path = args.output / f"{args.subject}_{args.label}_{int(time.time())}.csv"
    port = choose_port(args.port)
    print(f"Recording {args.label} from {port} to {path}")
    deadline = time.monotonic() + args.seconds
    count = 0
    with serial.Serial(port, 115200, timeout=1) as device, path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["t_ms", "ax", "ay", "az", "gx", "gy", "gz",
                         "state", "rep_count", "quality", "label", "subject"])
        while time.monotonic() < deadline:
            sample = parse_telemetry_line(device.readline().decode("utf-8", "ignore"))
            if sample is None:
                continue
            writer.writerow([sample.timestamp_ms, *sample.accel_g, *sample.gyro_dps,
                             sample.state, sample.rep_count, sample.quality,
                             args.label, args.subject])
            count += 1
    print(f"Saved {count} samples")


if __name__ == "__main__":
    main()
