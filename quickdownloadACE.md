```
git clone --recurse-submodules https://github.com/ServeurpersoCom/acestep.cpp
cd acestep.cpp

pip install huggingface_hub
./models.sh           # downloads Q8_0 turbo essentials (~7.7 GB)
./models.sh --sft     # downloads SFT model for higher quality (slower)

mkdir build && cd build
cmake ..
cmake --build . --config Release -j$(nproc)
cd ..

cat > ./request.json << 'EOF'
{
    "caption": "Fat techno track",
    "lyrics":             "[Instrumental]",
    "bpm":                174,
    "duration":           10,
    "keyscale":           "C minor",
    "timesignature":      "4",
    "vocal_language":     "unknown",
    "seed":               -1,
    "lm_temperature":     0.8,
    "lm_cfg_scale":       2.5,
    "lm_top_p":           0.9,
    "lm_top_k":           0,
    "lm_negative_prompt": "vocals, singing, melody, piano, guitar, strings, soft, ambient, slow",
    "audio_codes":        "",
    "inference_steps":    50,
    "guidance_scale":     7.0,
    "shift":              6.0
}
EOF

# LLM: generate lyrics + audio codes (Q8_0 = best quality)
./build/ace-qwen3 \
    --request ./request.json \
    --model models/acestep-5Hz-lm-4B-Q8_0.gguf

# DiT + VAE: synthesize audio (SFT model, 50 steps, much higher quality)
./build/dit-vae \
    --request ./request0.json \
    --text-encoder models/Qwen3-Embedding-0.6B-Q8_0.gguf \
    --dit models/acestep-v15-sft-Q8_0.gguf \
    --vae models/vae-BF16.gguf

# --- FAST VERSION (turbo, 8 steps, lower quality but ~6x faster) ---
# Change request.json: inference_steps=8, guidance_scale=0.0, shift=3.0
# Use: --dit models/acestep-v15-turbo-Q8_0.gguf
```


```
# LLM: generate lyrics + audio codes
./build/ace-qwen3 \
    --request ./request.json \
    --model models/acestep-5Hz-lm-4B-Q5_K_M.gguf

# DiT + VAE: synthesize audio
./build/dit-vae \
    --request ./request0.json \
    --text-encoder models/Qwen3-Embedding-0.6B-Q8_0.gguf \
    --dit models/acestep-v15-turbo-Q4_K_M.gguf \
    --vae models/vae-BF16.gguf
```