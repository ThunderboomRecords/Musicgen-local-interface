#!/bin/bash
export MACOSX_DEPLOYMENT_TARGET=12.0

UNAME_S=$(uname -s)
UNAME_M=$(uname -m)

export PKG_CONFIG_PATH="/opt/homebrew/opt/qt/lib/pkgconfig:$PKG_CONFIG_PATH"

if [ "$UNAME_S" = "Linux" ]; then
    make 
    cd ../../bin
    sudo rm -rf ../../vst2/MusicGenVST.vst
    sudo cp -r ../../bin/MusicGenVST.vst ../../vst2/MusicGenVST.vst
    rm -rf ../../vst3/MusicGenVST.vst3
    sudo cp -r ../../bin/MusicGenVST.vst3 ../../vst3/MusicGenVST.vst3
    ../../bin/MusicGenVST

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

    echo "signing"
    # Developer ID Application: Stichting thunderboom Records (M7XQ2U7M47)
    codesign --deep --force --sign - MusicGenVST.vst/Contents/MacOS/MusicGenVST
    codesign --deep --force --sign - MusicGenVST.app/Contents/MacOS/MusicGenVST
    codesign --deep --force --sign - MusicGenVST.vst3/Contents/MacOS/MusicGenVST

    if [ "$UNAME_M" = "arm64" ]; then
        sudo cp -r MusicGenVST.vst ../vst2/MusicGenVST.vst
        cp -r MusicGenVST.vst3 ../vst3/MusicGenVST.vst3
    fi

elif [ "$OS" = "Windows_NT" ]; then
    make 
    ../../bin/MusicGenVST
else
    echo "Unsupported operating system: $UNAME_S"
    exit 1
fi
