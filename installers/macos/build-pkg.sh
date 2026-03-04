#!/bin/bash
set -euo pipefail

# Usage: ./build-pkg.sh <PluginName> [Version]
# Example: ./build-pkg.sh MusicGenVST 1.0.0

PLUGIN="${1:?Usage: $0 <PluginName> [Version]}"
VERSION="${2:-0.0.1}"
ARTIFACT_DIR="build-artifacts"
OUTPUT_DIR="output"
STAGING_DIR="staging-${PLUGIN}"
PLUGIN_LOWER="$(echo "$PLUGIN" | tr '[:upper:]' '[:lower:]')"

mkdir -p "$OUTPUT_DIR" "$STAGING_DIR"

# --- Stage VST3 ---
VST3_ROOT="$STAGING_DIR/vst3/Library/Audio/Plug-Ins/VST3"
mkdir -p "$VST3_ROOT"
src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/VST3/${PLUGIN}.vst3"
if [ -d "$src" ]; then
    cp -R "$src" "$VST3_ROOT/"
fi

# --- Stage AU ---
AU_ROOT="$STAGING_DIR/au/Library/Audio/Plug-Ins/Components"
mkdir -p "$AU_ROOT"
src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/AU/${PLUGIN}.component"
if [ -d "$src" ]; then
    cp -R "$src" "$AU_ROOT/"
fi

# --- Stage Standalone ---
APP_ROOT="$STAGING_DIR/standalone/Applications/ThunderboomRecords"
mkdir -p "$APP_ROOT"
src="$ARTIFACT_DIR/build/Plugins/$PLUGIN/${PLUGIN}_artefacts/Release/Standalone/${PLUGIN}.app"
if [ -d "$src" ]; then
    cp -R "$src" "$APP_ROOT/"
fi

# --- Ad-hoc codesign bundles ---
for bundle in \
    "$VST3_ROOT/${PLUGIN}.vst3" \
    "$AU_ROOT/${PLUGIN}.component" \
    "$APP_ROOT/${PLUGIN}.app"; do
    if [ -d "$bundle" ]; then
        codesign --force --deep --sign - "$bundle"
        echo "Signed: $bundle"
    fi
done

# --- Build component packages ---
pkgbuild --root "$STAGING_DIR/vst3" \
         --identifier "com.thunderboomrecords.${PLUGIN_LOWER}.vst3" \
         --version "$VERSION" \
         "$STAGING_DIR/vst3.pkg"

pkgbuild --root "$STAGING_DIR/au" \
         --identifier "com.thunderboomrecords.${PLUGIN_LOWER}.au" \
         --version "$VERSION" \
         "$STAGING_DIR/au.pkg"

pkgbuild --root "$STAGING_DIR/standalone" \
         --identifier "com.thunderboomrecords.${PLUGIN_LOWER}.standalone" \
         --version "$VERSION" \
         "$STAGING_DIR/standalone.pkg"

# --- Generate distribution.xml ---
cat > "$STAGING_DIR/distribution.xml" << DISTXML
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
    <title>MusicGenVST v${VERSION} — macOS Universal</title>
    <organization>com.thunderboomrecords</organization>
    <welcome file="welcome.html"/>

    <options customize="allow" require-scripts="false" hostArchitectures="x86_64,arm64"/>

    <choices-outline>
        <line choice="vst3"/>
        <line choice="au"/>
        <line choice="standalone"/>
    </choices-outline>

    <choice id="vst3"
            title="VST3"
            description="Install ${PLUGIN} VST3 to /Library/Audio/Plug-Ins/VST3/">
        <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.vst3"/>
    </choice>

    <choice id="au"
            title="Audio Unit"
            description="Install ${PLUGIN} AU to /Library/Audio/Plug-Ins/Components/">
        <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.au"/>
    </choice>

    <choice id="standalone"
            title="Standalone"
            description="Install ${PLUGIN} standalone app to /Applications/ThunderboomRecords/">
        <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.standalone"/>
    </choice>

    <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.vst3" version="${VERSION}">vst3.pkg</pkg-ref>
    <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.au" version="${VERSION}">au.pkg</pkg-ref>
    <pkg-ref id="com.thunderboomrecords.${PLUGIN_LOWER}.standalone" version="${VERSION}">standalone.pkg</pkg-ref>
</installer-gui-script>
DISTXML

# --- Build final product installer ---
productbuild --distribution "$STAGING_DIR/distribution.xml" \
             --package-path "$STAGING_DIR" \
             --resources installers/macos/resources \
             "$OUTPUT_DIR/MusicGenVST-${VERSION}-macOS.pkg"

# Clean up staging
rm -rf "$STAGING_DIR"

echo "Created: $OUTPUT_DIR/MusicGenVST-${VERSION}-macOS.pkg"
