#!/bin/bash
# Download ACE-Step GGUF models from HuggingFace using curl
# No external dependencies required (no hf CLI / pip)
#
# Usage: DIR=/path/to/models ./download-models.sh

set -eu

REPO="Serveurperso/ACE-Step-1.5-GGUF"
BASE_URL="https://huggingface.co/$REPO/resolve/main"
DIR="${DIR:-models}"

mkdir -p "$DIR"

MODELS=(
    "vae-BF16.gguf"
    "Qwen3-Embedding-0.6B-Q8_0.gguf"
    "acestep-5Hz-lm-4B-Q8_0.gguf"
    "acestep-v15-turbo-Q8_0.gguf"
    "acestep-v15-sft-Q8_0.gguf"
)

for model in "${MODELS[@]}"; do
    outpath="$DIR/$model"
    if [ -f "$outpath" ]; then
        echo "[OK] $model"
        continue
    fi
    echo "[Download] $model"
    curl -L --progress-bar -o "$outpath" "$BASE_URL/$model" || {
        echo "[Error] Failed to download $model"
        rm -f "$outpath"
    }
done

echo "[Done] Models ready in $DIR/"
