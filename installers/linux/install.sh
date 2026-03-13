#!/bin/bash
set -euo pipefail

# MusicGenVST — Linux Install Script
# Usage: ./install.sh [--vst3-path DIR] [--bin-path DIR]

VST3_DIR="${HOME}/.vst3"
BIN_DIR="${HOME}/.local/bin"
ACESTEP_DIR="${HOME}/.local/share/MusicGenVST/acestep"
MODELS_DIR="${HOME}/.local/share/MusicGenVST/models"

while [[ $# -gt 0 ]]; do
    case $1 in
        --vst3-path) VST3_DIR="$2"; shift 2 ;;
        --bin-path)  BIN_DIR="$2"; shift 2 ;;
        --help)
            echo "MusicGenVST Plugin Installer"
            echo ""
            echo "Usage: ./install.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --vst3-path DIR  Install VST3 plugin here (default: ~/.vst3)"
            echo "  --bin-path DIR   Install standalone app here (default: ~/.local/bin)"
            echo "  --help           Show this help message"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "MusicGenVST Plugin Installer"
echo "============================"

if [ -d "$SCRIPT_DIR/vst3" ]; then
    echo "Installing VST3 plugin to $VST3_DIR ..."
    mkdir -p "$VST3_DIR"
    cp -R "$SCRIPT_DIR/vst3/"*.vst3 "$VST3_DIR/"
fi

if [ -d "$SCRIPT_DIR/standalone" ] && [ "$(ls -A "$SCRIPT_DIR/standalone" 2>/dev/null)" ]; then
    echo "Installing standalone app to $BIN_DIR ..."
    mkdir -p "$BIN_DIR"
    cp "$SCRIPT_DIR/standalone/"* "$BIN_DIR/"
    chmod +x "$BIN_DIR/"*
fi

# Install ACE-Step binaries
if [ -d "$SCRIPT_DIR/acestep" ]; then
    echo "Installing ACE-Step binaries to $ACESTEP_DIR ..."
    mkdir -p "$ACESTEP_DIR"
    cp "$SCRIPT_DIR/acestep/ace-qwen3" "$ACESTEP_DIR/" 2>/dev/null || true
    cp "$SCRIPT_DIR/acestep/dit-vae" "$ACESTEP_DIR/" 2>/dev/null || true
    chmod +x "$ACESTEP_DIR/"* 2>/dev/null || true

    # Download models
    if [ -f "$SCRIPT_DIR/acestep/download-models.sh" ]; then
        echo "Downloading ACE-Step models to $MODELS_DIR ..."
        mkdir -p "$MODELS_DIR"
        DIR="$MODELS_DIR" bash "$SCRIPT_DIR/acestep/download-models.sh" || echo "Warning: Model download failed. Run download-models.sh manually later."
    fi
fi

echo ""
echo "Installation complete."
