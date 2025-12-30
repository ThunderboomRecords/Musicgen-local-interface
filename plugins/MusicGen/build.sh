#!/bin/bash
export MACOSX_DEPLOYMENT_TARGET=12.0

UNAME_S=$(uname -s)
UNAME_M=$(uname -m)

export PKG_CONFIG_PATH="/opt/homebrew/opt/qt/lib/pkgconfig:$PKG_CONFIG_PATH"

# Load the certificate identity from a separate file if it exists
CERT_FILE="../../codesign_cert.txt"
if [ -f "$CERT_FILE" ]; then
    CODESIGN_IDENTITY=$(cat "$CERT_FILE")
else
    CODESIGN_IDENTITY="-"
fi

mkdir ../../release
if [ "$UNAME_S" = "Linux" ]; then
    echo "Running on Linux amd64"
    make 

    cd ../../bin
    sudo rm -rf ../../vst2/MusicGenVST.vst
    sudo cp -r ../../bin/MusicGenVST.vst ../../vst2/MusicGenVST.vst
    rm -rf ../../vst3/MusicGenVST.vst3
    sudo cp -r ../../bin/MusicGenVST.vst3 ../../vst3/MusicGenVST.vst3

    zip -r standalone-x86_64-linux.zip MusicGenVST
    zip -r vst2-x86_64-linux.zip MusicGenVST.vst
    zip -r vst3-x86_64-linux.zip MusicGenVST.vst3

    mv ./standalone-x86_64-linux.zip ../release
    mv ./vst2-x86_64-linux.zip ../release
    mv ./vst3-x86_64-linux.zip ../release

elif [ "$UNAME_S" = "Darwin" ]; then
    sudo rm -rf ../../vst2/MusicGenVST.vst
    rm -rf ../../vst3/MusicGenVST.vst3
    if [ "$UNAME_M" = "x86_64" ]; then
        echo "Running on macOS amd64"
        echo $MACOSX_DEPLOYMENT_TARGET

        arch -x86_64 make

        ARCH_NAME="amd64"
    elif [ "$UNAME_M" = "arm64" ]; then
        echo "Running on macOS arm64"
        echo $MACOSX_DEPLOYMENT_TARGET

        arch -arm64 make

        ARCH_NAME="arm64"
    fi
    
    cd ../../bin/

    dylibbundler -od -b -x MusicGenVST.app/Contents/MacOS/MusicGenVST \
    -d MusicGenVST.app/Contents/Frameworks \
    -p @executable_path/../Frameworks
    dylibbundler -od -b -x MusicGenVST.vst/Contents/MacOS/MusicGenVST \
    -d MusicGenVST.vst/Contents/Frameworks \
    -p @loader_path/../Frameworks
    dylibbundler -od -b -x MusicGenVST.vst3/Contents/MacOS/MusicGenVST \
    -d MusicGenVST.vst3/Contents/Frameworks \
    -p @loader_path/../Frameworks

    echo "signing"
    # Developer ID Application: Stichting thunderboom Records (M7XQ2U7M47)
    codesign --deep --force --sign "$CODESIGN_IDENTITY" MusicGenVST.vst/Contents/MacOS/MusicGenVST
    codesign --deep --force --sign "$CODESIGN_IDENTITY" MusicGenVST.app/Contents/MacOS/MusicGenVST
    codesign --deep --force --sign "$CODESIGN_IDENTITY" MusicGenVST.vst3/Contents/MacOS/MusicGenVST

    zip -r standalone-$ARCH_NAME-osx.zip MusicGenVST.app
    zip -r vst2-$ARCH_NAME-osx.zip MusicGenVST.vst
    zip -r vst3-$ARCH_NAME-osx.zip MusicGenVST.vst3
    xcrun notarytool submit vst2-$ARCH_NAME-osx.zip --apple-id "bjmaat@gmail.com" --password "ogxv-bfta-oshc-tmyo" --team-id "M7XQ2U7M47" --wait
    xcrun notarytool submit vst3-$ARCH_NAME-osx.zip --apple-id "bjmaat@gmail.com" --password "ogxv-bfta-oshc-tmyo" --team-id "M7XQ2U7M47" --wait

    echo "Stapling vst"
    xcrun stapler staple MusicGenVST.vst
    
    echo "Stapling vst3"
    xcrun stapler staple MusicGenVST.vst3

    rm -rf ./vst2-$ARCH_NAME-osx.zip
    rm -rf ./vst3-$ARCH_NAME-osx.zip

    sudo cp -r MusicGenVST.vst ../vst2/MusicGenVST.vst
    cp -r MusicGenVST.vst3 ../vst3/MusicGenVST.vst3
    
    zip -r vst2-$ARCH_NAME-osx.zip MusicGenVST.vst
    zip -r vst3-$ARCH_NAME-osx.zip MusicGenVST.vst3

    mv standalone-$ARCH_NAME-osx.zip ../release/
    mv vst2-$ARCH_NAME-osx.zip ../release/
    mv vst3-$ARCH_NAME-osx.zip ../release/

    if [ "$UNAME_M" = "arm64" ]; then
        sudo cp -r MusicGenVST.vst ../vst2/MusicGenVST.vst
        cp -r MusicGenVST.vst3 ../vst3/MusicGenVST.vst3
    fi

elif [ "$OS" = "Windows_NT" ]; then
    echo "Running on Linux amd64"
    make 
    ../../bin/MusicGenVST
else
    echo "Unsupported operating system: $UNAME_S"
    exit 1
fi
