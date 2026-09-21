#!/bin/bash
set -e

sudo pacman -Syu --needed --noconfirm --noprogressbar base-devel cmake git
sudo pacman -Syu --needed --noconfirm --noprogressbar qt5-base qt5-declarative \
    qt5-quickcontrols2 qt5-websockets qt5-webchannel qt5-tools

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

printf 'Building source: %s\n' "$SCRIPT_DIR"
if command -v git >/dev/null 2>&1 && git -C "$SCRIPT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'Source commit: %s\n' "$(git -C "$SCRIPT_DIR" rev-parse HEAD)"
fi

rm -rf "$BUILD_DIR"
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
