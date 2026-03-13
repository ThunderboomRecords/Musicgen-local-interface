#!/bin/bash

JOBS=$(sysctl -n hw.ncpu)

cmake -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release -j"$JOBS" 2>&1
sudo cp -r ./build/plugins/MusicGenVST/MusicGenVST_artefacts/Release/VST3/MusicGenVST.vst3 /Library/Audio/Plug-Ins/VST3/
sudo codesign --force --sign - /Library/Audio/Plug-Ins/VST3/MusicGenVST.vst3
