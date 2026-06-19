#!/bin/sh
set -e

echo "============================================="
echo " Installing Sysmon (Interactive System Monitor)"
echo "============================================="

# Ensure prerequisites exist
if ! command -v git >/dev/null 2>&1; then
    echo "Error: git is required but not installed."
    exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
    echo "Error: cmake is required but not installed."
    exit 1
fi
if ! command -v make >/dev/null 2>&1; then
    echo "Error: make is required but not installed."
    exit 1
fi

TMP_DIR=$(mktemp -d -t sysmon-install-XXXXXX)
echo "=> Cloning repository into temporary directory..."
git clone --depth 1 https://github.com/mukundangopalachary/sysmon.git "$TMP_DIR"

echo "=> Building from source..."
cd "$TMP_DIR"
mkdir build
cd build
cmake ..
make

echo "=> Installing globally (requires sudo)..."
sudo make install

echo "=> Cleaning up..."
rm -rf "$TMP_DIR"

echo "============================================="
echo " Installation Complete!"
echo " You can now run 'sysmon' from anywhere."
echo "============================================="
