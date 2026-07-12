from __future__ import annotations

import numpy as np


def make_windows(data: np.ndarray, window_size: int = 100, stride: int = 50) -> np.ndarray:
    data = np.asarray(data, dtype=np.float32)
    if data.ndim != 2:
        raise ValueError("data must have shape [samples, channels]")
    if window_size <= 0 or stride <= 0:
        raise ValueError("window_size and stride must be positive")
    if len(data) < window_size:
        return np.empty((0, window_size, data.shape[1]), dtype=np.float32)
    starts = range(0, len(data) - window_size + 1, stride)
    return np.stack([data[start : start + window_size] for start in starts])
