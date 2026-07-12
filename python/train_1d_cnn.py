#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
import torch
from torch.utils.data import DataLoader, TensorDataset

from literehab.cnn import build_model
from literehab.dataset import make_windows


FEATURES = ["ax", "ay", "az", "gx", "gy", "gz"]


def load_recording(path: Path):
    with path.open(newline="") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        return None
    data = np.asarray([[float(row[name]) for name in FEATURES] for row in rows],
                      dtype=np.float32)
    return data, rows[0]["label"], rows[0]["subject"]


def main() -> None:
    parser = argparse.ArgumentParser(description="Train the LiteRehab IMU 1D-CNN")
    parser.add_argument("--data", type=Path, default=Path("data"))
    parser.add_argument("--holdout-subject", required=True)
    parser.add_argument("--epochs", type=int, default=30)
    parser.add_argument("--arch", choices=["cnn_1d", "cnn_bigru"],
                        default="cnn_bigru")
    parser.add_argument("--output", type=Path, default=Path("models/imu_1dcnn.pt"))
    args = parser.parse_args()

    recordings = [load_recording(path) for path in sorted(args.data.glob("*.csv"))]
    recordings = [item for item in recordings if item is not None]
    if not recordings:
        raise SystemExit("No labeled CSV recordings found")
    labels = sorted({label for _, label, _ in recordings})
    label_to_index = {label: i for i, label in enumerate(labels)}
    train_x, train_y, test_x, test_y = [], [], [], []
    for data, label, subject in recordings:
        windows = make_windows(data, 100, 50)
        target_x, target_y = ((test_x, test_y) if subject == args.holdout_subject
                              else (train_x, train_y))
        target_x.extend(windows)
        target_y.extend([label_to_index[label]] * len(windows))
    if not train_x or not test_x:
        raise SystemExit("Both training subjects and the held-out subject need recordings")

    train_array = np.asarray(train_x, dtype=np.float32)
    mean = train_array.mean(axis=(0, 1), keepdims=True)
    std = train_array.std(axis=(0, 1), keepdims=True) + 1e-6
    train_array = (train_array - mean) / std
    test_array = (np.asarray(test_x, dtype=np.float32) - mean) / std

    def tensor_x(values):
        return torch.tensor(values).permute(0, 2, 1)

    loader = DataLoader(TensorDataset(tensor_x(train_array), torch.tensor(train_y)),
                        batch_size=32, shuffle=True)
    model = build_model(len(labels), arch=args.arch)
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)
    criterion = torch.nn.CrossEntropyLoss()
    for epoch in range(args.epochs):
        model.train()
        total = 0.0
        for features, targets in loader:
            optimizer.zero_grad()
            loss = criterion(model(features), targets)
            loss.backward()
            optimizer.step()
            total += loss.item() * len(targets)
        print(f"epoch={epoch + 1} loss={total / len(train_array):.4f}")

    model.eval()
    with torch.no_grad():
        predictions = model(tensor_x(test_array)).argmax(dim=1)
    accuracy = (predictions == torch.tensor(test_y)).float().mean().item()
    print(f"held_out_subject={args.holdout_subject} accuracy={accuracy:.3f}")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    torch.save({"state_dict": model.state_dict(), "labels": labels,
                "mean": mean.reshape(-1), "std": std.reshape(-1),
                "window_size": 100, "arch": args.arch,
                "held_out_subject": args.holdout_subject,
                "accuracy": accuracy}, args.output)
    print(f"saved={args.output}")


if __name__ == "__main__":
    main()
