# MusicGenVST

A VST plugin implementation using MusicGen.

## Prerequisites

Before building the project, ensure your system has the necessary build tools and libraries installed.

### System Dependencies (Linux)

Update your package manager and install the core build tools:

```bash
sudo apt update
sudo apt install build-essential git cmake g++ pkg-config meson

```

### Library Dependencies

Install the required development libraries for graphics, audio, and protobuf support:

```bash
sudo apt install libx11-dev libgl1-mesa-dev libasound2-dev libjack-jackd2-dev libprotobuf-dev protobuf-compiler

```

---

## Installation

### 1. Clone the Repository

Clone this repository and initialize the required submodules:

```bash
git clone [https://github.com/YOUR_USERNAME/MusicGenVST.git](https://github.com/YOUR_USERNAME/MusicGenVST.git)
cd MusicGenVST
git submodule update --init --recursive

```

### 2. Install/Build External Dependencies

#### DPF (DISTRHO Plugin Framework)

```bash
git clone [https://github.com/DISTRHO/DPF.git](https://github.com/DISTRHO/DPF.git)

```

#### ONNX Runtime

This project requires `onnxruntime` to be built from source.

```bash
# Install ONNX build dependencies
sudo apt-get install build-essential libprotobuf-dev protobuf-compiler git cmake

# Clone the repository
git clone [https://github.com/microsoft/onnxruntime.git](https://github.com/microsoft/onnxruntime.git)
cd onnxruntime

# Build shared libraries
./build.sh --config Release --build_shared_lib --parallel

# Install to system
sudo ./build.sh --build_shared_lib --parallel --install

# Return to project root
cd ..

```

---

## Building

### Linking

Ensure you link against the ONNX Runtime correctly during compilation. An example build command looks like this:

```bash
g++ -I/usr/local/include/onnxruntime -L/usr/local/lib -lonnxruntime main.cpp -o MusicGenVST

```

---

## Configuration & Usage

### Environment Variables (Linux)

You may need to update your library path to locate the built shared libraries for ONNX Runtime and SentencePiece before running the application.

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./src/onnxruntime
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./src/sentencepiece

```

> **Note:** Windows build instructions are currently in development.

---

## Acknowledgments & Libraries

* [DPF](https://github.com/DISTRHO/DPF.git)
* [ONNX Runtime](https://github.com/microsoft/onnxruntime.git)
* [nlohmann/json](https://github.com/nlohmann/json)
* [SentencePiece](https://github.com/google/sentencepiece)
