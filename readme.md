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
git clone https://github.com/YOUR_USERNAME/MusicGenVST.git
cd MusicGenVST
git submodule update --init --recursive
```

### 2. Install/Build External Dependencies

#### DPF (DISTRHO Plugin Framework)

```bash
git clone https://github.com/DISTRHO/DPF.git
```

#### ONNX Runtime

This project requires `onnxruntime` to be built from source.

```bash
# Install ONNX build dependencies
sudo apt-get install build-essential libprotobuf-dev protobuf-compiler git cmake

# Clone the repository
git clone https://github.com/microsoft/onnxruntime.git
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

## Local Server Setup (Python)

This VST communicates with a local Python backend (`app.py`) to perform the AI audio generation. You must have this server running in the background for the plugin to function.

### 1. Server Prerequisites

* **Python 3.9** or higher.
* **FFmpeg**: Required for audio processing.
  * Linux: `sudo apt install ffmpeg`
  * Windows: Download FFmpeg and add to PATH.
  * Mac: `brew install ffmpeg`

### 2. Python Environment Setup

It is recommended to use a virtual environment. Navigate to the folder containing `app.py`:

```bash
# Create virtual environment
python -m venv venv

# Activate environment
# Windows: .\venv\Scripts\activate
# Linux/Mac: source venv/bin/activate
```

### 3. Install Python Dependencies

Install PyTorch and the required libraries.

**Note for GPU Users:** To enable fast generation, ensure you install the CUDA version of PyTorch compatible with your hardware. See pytorch.org.

```bash
# Basic installation (CPU)
pip install torch torchvision torchaudio

# Install remaining requirements
pip install flask transformers soundfile scipy
```

#### 3.1
Predownload the models in a .cache dir (will make it easier later when running the Docker Container):
```python ./download.py```

### 4. Running the Server

Start the Flask application. It will listen on port **55000**.

```bash
python app.py
```

> **First Run Note:** When you run the server for the first time, it will automatically download the MusicGen models (approx. 2-3 GB). Please wait for the download to finish before using the VST.

### 5. Docker Usage (Optional)

If you prefer to run the backend in a container, build and run the provided Dockerfile:

```bash
docker build -t musicgen-server .
docker run -p 55000:5000 musicgen-server
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
* [Hugging Face Transformers](https://github.com/huggingface/transformers)
* [Flask](https://github.com/pallets/flask)
