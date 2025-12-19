#!/usr/bin/env bash
#
# Build Android artifacts for MaYaRa Server Plugin
# Based on OpenCPN plugin template (shipdriver_pi)
#

set -xe

# Required environment variable
: ${OCPN_TARGET:?}

# Install build dependencies
sudo apt-get -qq update
sudo apt-get -qq install -y cmake git gettext

# Install newer cmake via pip (the packaged one is too old for NDK)
python3 -m pip install -q --user cmake
export PATH="$HOME/.local/bin:$PATH"

# Install cloudsmith CLI for uploads
python3 -m pip install -q --user cloudsmith-cli

# Set up Android NDK - find the most recent one in CircleCI environment
NDK_HOME=$(ls -d /opt/android/android-ndk-* 2>/dev/null | tail -1) || true
if [ -z "$NDK_HOME" ]; then
    NDK_HOME=$(ls -d $ANDROID_NDK_HOME 2>/dev/null | tail -1) || true
fi
if [ -z "$NDK_HOME" ]; then
    NDK_HOME=$(ls -d ~/Android/Sdk/ndk/* 2>/dev/null | tail -1) || true
fi

if [ -z "$NDK_HOME" ]; then
    echo "ERROR: Cannot find Android NDK"
    exit 1
fi

echo "Using NDK: $NDK_HOME"

# Create symlink expected by toolchain files
sudo mkdir -p /opt/android
sudo ln -sf "$NDK_HOME" /opt/android/ndk

# Create build directory
rm -rf build && mkdir build && cd build

# Configure and build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/${OCPN_TARGET}-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      ..

make -j$(nproc) VERBOSE=1

# Create tarball
make tarball

echo "Build complete. Artifacts:"
ls -la *.tar.gz *.xml 2>/dev/null || echo "No artifacts found"
