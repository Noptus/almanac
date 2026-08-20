#!/usr/bin/env bash
# Build the desktop preview harness. Regenerates the C++ fragment bank from the
# Python source of truth first, so the two never drift.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "[1/3] generating C++ fragment bank from Python source of truth..."
python3 tools/gen_fragments_cpp.py

echo "[2/3] embedding EB Garamond serif (ASCII subset)..."
python3 tools/gen_font_cpp.py

echo "[3/3] compiling oracle_cpp..."
c++ -std=c++17 -O2 -Wall -Wextra -o oracle_cpp \
    firmware/main.cpp \
    firmware/src/oracle_astro.cpp \
    firmware/src/oracle_composer.cpp \
    firmware/src/oracle_render.cpp \
    firmware/src/oracle_text.cpp

echo "done. try:"
echo "  ./oracle_cpp --birth 1990-06-15 --name Sam --place Lyon --crystal amethyst --date 2026-08-20 --out reading.bmp"
