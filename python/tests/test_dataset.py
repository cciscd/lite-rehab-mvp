import numpy as np

from literehab.dataset import make_windows


def test_makes_two_second_windows_with_half_overlap():
    data = np.arange(250 * 6, dtype=np.float32).reshape(250, 6)
    windows = make_windows(data, window_size=100, stride=50)
    assert windows.shape == (4, 100, 6)
    np.testing.assert_array_equal(windows[1, 0], data[50])


def test_short_recording_returns_empty_window_array():
    data = np.zeros((20, 6), dtype=np.float32)
    windows = make_windows(data, window_size=100, stride=50)
    assert windows.shape == (0, 100, 6)
