# MusicGenVST

An open-source VST/AU MIDI plugin for AI music generation, built with [JUCE](https://juce.com) and [ACE-Step](https://github.com/AceStep/ACE-Step).

## Prerequisites

### All Platforms

- **CMake** 3.22+
- **Ninja** build system
- **Git** (with submodule support)

### Linux

```bash
sudo apt-get update
sudo apt-get install -y build-essential git cmake ninja-build \
  libasound2-dev libx11-dev libxinerama-dev libxext-dev \
  libfreetype6-dev libwebkit2gtk-4.1-dev libglu1-mesa-dev \
  libcurl4-openssl-dev libxrandr-dev libfontconfig1-dev \
  libxcursor-dev libgtk-3-dev
```

### macOS

```bash
brew install ninja cmake
```

### Windows

```powershell
choco install ninja cmake -y
```

Requires MSVC (Visual Studio Build Tools).

---

## Building

### 1. Clone the Repository

```bash
git clone https://github.com/thunderboomrecords/MusicGenVST.git
cd MusicGenVST
git submodule update --init --recursive
```

### 2. Build ACE-Step

```bash
cd acestep.cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cd ..
```

On macOS, for a universal binary add `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`.

### 3. Build the Plugin

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target MusicGenVST_VST3 --config Release --parallel
```

On macOS you can also build the AU target:

```bash
cmake --build build --target MusicGenVST_AU --config Release --parallel
```

Build artifacts are output to `build/Plugins/MusicGenVST/MusicGenVST_artefacts/Release/`.

---

## CI / Releases

Builds are automated via GitHub Actions. Tags control what gets built:

| Tag pattern | Example | Effect |
|---|---|---|
| `beta-[os]-v[version]` | `beta-all-v1.0.0` | Build only (no release) |
| `release-[os]-v[version]` | `release-all-v2.0.0` | Build + GitHub Release |

The `[os]` segment can be `all`, `macos`, `windows`, or `linux`.

Installers are produced per platform: `.pkg` (macOS), `.exe` (Windows), `.deb` / `.tar.gz` (Linux).

---

## Acknowledgments & Libraries

* [JUCE](https://juce.com) — Audio plugin framework
* [ACE-Step (acestep.cpp)](https://github.com/AceStep/ACE-Step) — AI music generation inference
* [nlohmann/json](https://github.com/nlohmann/json) — JSON for C++
