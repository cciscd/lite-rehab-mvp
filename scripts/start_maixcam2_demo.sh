#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PYTHON=${PYTHON:-/opt/anaconda3/bin/python3.13}
SOURCE=${1:-auto}

PYTHONPATH="$ROOT/python" "$PYTHON" "$ROOT/python/run_dashboard.py" \
  --port auto \
  --camera-source "$SOURCE" \
  --side right \
  --output "$ROOT/python/sessions/maixcam2_demo.csv"

