#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# Reuse the offline JUCE 8.0.8 checkout and the rootless Linux dev sysroot that
# already exist next to this project. No network access is required locally.
SIBLING="${SIBLING_POC:-/home/brians/bls/sound/plugin_poc}"

if [ ! -e external/JUCE/CMakeLists.txt ]; then
    if [ -f "$SIBLING/external/JUCE/CMakeLists.txt" ]; then
        mkdir -p external
        ln -sfn "$SIBLING/external/JUCE" external/JUCE
    else
        echo "No local JUCE checkout found; CMake will fetch JUCE 8.0.8."
    fi
fi

if [ ! -d deps/sysroot/usr/include ]; then
    if [ -d "$SIBLING/deps/sysroot/usr/include" ]; then
        mkdir -p deps
        ln -sfn "$SIBLING/deps/sysroot" deps/sysroot
    else
        echo "No offline sysroot found; using system build dependencies."
    fi
fi

echo "Dependencies are ready. Run scripts/build.sh next."
