#!/bin/bash
set -euo pipefail

# Usage: ./build-deb.sh <PluginName> [Version]
# Example: ./build-deb.sh MusicGenVST 1.0.0

PLUGIN="${1:?Usage: $0 <PluginName> [Version]}"
VERSION="${2:-0.0.1}"
ARTIFACT_DIR="build-artifacts"
OUTPUT_DIR="output"
PLUGIN_LOWER="$(echo "$PLUGIN" | tr '[:upper:]' '[:lower:]')"
DEB_ROOT="deb-staging/${PLUGIN_LOWER}_${VERSION}_amd64"

mkdir -p "$OUTPUT_DIR"

# =====================
# .deb package
# =====================
mkdir -p "$DEB_ROOT/DEBIAN"
mkdir -p "$DEB_ROOT/usr/lib/vst3"
mkdir -p "$DEB_ROOT/usr/local/bin"
mkdir -p "$DEB_ROOT/usr/lib/musicgenvst/acestep"

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3"
if [ -d "$src" ]; then
    cp -R "$src" "$DEB_ROOT/usr/lib/vst3/"
fi

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/Standalone/$PLUGIN"
if [ -f "$src" ]; then
    cp "$src" "$DEB_ROOT/usr/local/bin/"
fi

# Bundle ACE-Step binaries and models.sh
for bin in ace-qwen3 dit-vae; do
    if [ -f "$ARTIFACT_DIR/acestep.cpp/build/$bin" ]; then
        cp "$ARTIFACT_DIR/acestep.cpp/build/$bin" "$DEB_ROOT/usr/lib/musicgenvst/acestep/"
        chmod +x "$DEB_ROOT/usr/lib/musicgenvst/acestep/$bin"
    fi
done
if [ -f "$ARTIFACT_DIR/installers/download-models.sh" ]; then
    cp "$ARTIFACT_DIR/installers/download-models.sh" "$DEB_ROOT/usr/lib/musicgenvst/acestep/"
    chmod +x "$DEB_ROOT/usr/lib/musicgenvst/acestep/download-models.sh"
fi

cat > "$DEB_ROOT/DEBIAN/control" << EOF
Package: ${PLUGIN_LOWER}
Version: ${VERSION}
Section: sound
Priority: optional
Architecture: amd64
Maintainer: Thunderboom Records <info@thunderboomrecords.com>
Description: MusicGenVST v${VERSION} — Linux amd64 VST3 MIDI plugin
Depends: libasound2, libfreetype6, libx11-6, libxext6, libxinerama1
Homepage: https://github.com/Revess/MusicGenVST
EOF

# Postinst script to download models
cat > "$DEB_ROOT/DEBIAN/postinst" << 'POSTINST'
#!/bin/bash
MODELS_DIR="$HOME/.local/share/MusicGenVST/models"
DL_SCRIPT="/usr/lib/musicgenvst/acestep/download-models.sh"
if [ -f "$DL_SCRIPT" ]; then
    mkdir -p "$MODELS_DIR"
    DIR="$MODELS_DIR" bash "$DL_SCRIPT" || true
fi
exit 0
POSTINST
chmod 0755 "$DEB_ROOT/DEBIAN/postinst"

dpkg-deb --build "$DEB_ROOT" "$OUTPUT_DIR/${PLUGIN_LOWER}_${VERSION}_amd64.deb"

# =====================
# .tar.gz with install script
# =====================
TAR_ROOT="tar-staging/MusicGenVST-${VERSION}-Linux"
mkdir -p "$TAR_ROOT/vst3"
mkdir -p "$TAR_ROOT/standalone"
mkdir -p "$TAR_ROOT/acestep"

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3"
if [ -d "$src" ]; then
    cp -R "$src" "$TAR_ROOT/vst3/"
fi

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/Standalone/$PLUGIN"
if [ -f "$src" ]; then
    cp "$src" "$TAR_ROOT/standalone/"
fi

# Bundle ACE-Step binaries and models.sh
for bin in ace-qwen3 dit-vae; do
    if [ -f "$ARTIFACT_DIR/acestep.cpp/build/$bin" ]; then
        cp "$ARTIFACT_DIR/acestep.cpp/build/$bin" "$TAR_ROOT/acestep/"
        chmod +x "$TAR_ROOT/acestep/$bin"
    fi
done
if [ -f "$ARTIFACT_DIR/installers/download-models.sh" ]; then
    cp "$ARTIFACT_DIR/installers/download-models.sh" "$TAR_ROOT/acestep/"
    chmod +x "$TAR_ROOT/acestep/download-models.sh"
fi

cp installers/linux/install.sh "$TAR_ROOT/"

tar -czf "$OUTPUT_DIR/MusicGenVST-${VERSION}-Linux.tar.gz" \
    -C tar-staging "MusicGenVST-${VERSION}-Linux"

# Clean up staging
rm -rf "$DEB_ROOT" "$TAR_ROOT"

echo "Created: ${PLUGIN_LOWER}_${VERSION}_amd64.deb + MusicGenVST-${VERSION}-Linux.tar.gz"
