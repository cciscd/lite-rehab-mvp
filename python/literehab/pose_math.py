from __future__ import annotations

import math
from collections.abc import Mapping, Sequence

Point = Sequence[float]


def angle_degrees(a: Point, b: Point, c: Point) -> float | None:
    """Return angle ABC in degrees, or None when either arm has zero length."""
    bax, bay = a[0] - b[0], a[1] - b[1]
    bcx, bcy = c[0] - b[0], c[1] - b[1]
    len_ba = math.hypot(bax, bay)
    len_bc = math.hypot(bcx, bcy)
    if len_ba <= 1e-9 or len_bc <= 1e-9:
        return None
    cosine = (bax * bcx + bay * bcy) / (len_ba * len_bc)
    return math.degrees(math.acos(max(-1.0, min(1.0, cosine))))


def trunk_compensation(
    baseline: Mapping[str, Point],
    current: Mapping[str, Point],
    threshold: float = 0.15,
) -> bool:
    """Detect shoulder movement relative to the hip, normalized by torso length."""
    try:
        base_shoulder, base_hip = baseline["shoulder"], baseline["hip"]
        shoulder, hip = current["shoulder"], current["hip"]
    except (KeyError, TypeError):
        return False
    torso = math.hypot(
        base_shoulder[0] - base_hip[0], base_shoulder[1] - base_hip[1]
    )
    if torso <= 1e-9:
        return False
    base_relative = (
        base_shoulder[0] - base_hip[0], base_shoulder[1] - base_hip[1]
    )
    current_relative = (shoulder[0] - hip[0], shoulder[1] - hip[1])
    displacement = math.hypot(
        current_relative[0] - base_relative[0],
        current_relative[1] - base_relative[1],
    )
    return displacement / torso >= threshold
