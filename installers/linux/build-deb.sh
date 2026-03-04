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

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3"
if [ -d "$src" ]; then
    cp -R "$src" "$DEB_ROOT/usr/lib/vst3/"
fi

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/Standalone/$PLUGIN"
if [ -f "$src" ]; then
    cp "$src" "$DEB_ROOT/usr/local/bin/"
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

dpkg-deb --build "$DEB_ROOT" "$OUTPUT_DIR/${PLUGIN_LOWER}_${VERSION}_amd64.deb"

# =====================
# .tar.gz with install script
# =====================
TAR_ROOT="tar-staging/MusicGenVST-${VERSION}-Linux"
mkdir -p "$TAR_ROOT/vst3"
mkdir -p "$TAR_ROOT/standalone"

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3"
if [ -d "$src" ]; then
    cp -R "$src" "$TAR_ROOT/vst3/"
fi

src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/Standalone/$PLUGIN"
if [ -f "$src" ]; then
    cp "$src" "$TAR_ROOT/standalone/"
fi

cp installers/linux/install.sh "$TAR_ROOT/"

tar -czf "$OUTPUT_DIR/MusicGenVST-${VERSION}-Linux.tar.gz" \
    -C tar-staging "MusicGenVST-${VERSION}-Linux"

# Clean up staging
rm -rf "$DEB_ROOT" "$TAR_ROOT"

echo "Created: ${PLUGIN_LOWER}_${VERSION}_amd64.deb + MusicGenVST-${VERSION}-Linux.tar.gz"
