#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
. /Users/yuedonghan/.espressif/v6.0.2/esp-idf/export.sh >/dev/null 2>&1

idf.py -C "$ROOT/wearable" build
idf.py -C "$ROOT/receiver" build
