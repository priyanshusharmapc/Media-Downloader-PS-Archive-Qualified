#!/bin/bash
set -e

sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qtbase5-dev qtdeclarative5-dev \
    qml-module-qtquick-controls2 qml-module-qtwebsockets \
    qml-module-qtwebchannel qttools5-dev qttools5-dev-tools

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

printf 'Building source: %s\n' "$SCRIPT_DIR"
if command -v git >/dev/null 2>&1 && git -C "$SCRIPT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'Source commit: %s\n' "$(git -C "$SCRIPT_DIR" rev-parse HEAD)"
fi

rm -rf "$BUILD_DIR"
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --parallel "$(nproc)"
