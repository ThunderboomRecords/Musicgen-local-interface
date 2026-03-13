## Intergration of VST to use acestep.cpp
The general goal of this VST build is to:
1. Upgrade from MusicGen to acestep.
2. To revamp the vst and start using JUCE over DPF.

So what we want is that the user can add a prompt and instrumentation (vocals etc.) and presses generate with the currently available parameters:
CFG, temp, topk, topp, length in seconds, bpm and num samples.

Now since the use of acestep.cpp is so simple and fast in use would I like the following.

After the user has filled in the different steps and presses generate the VST should write a JSON file (like):
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
To the local VST folder

Then the VST should (lock-free, so in a seperate thread).
Spin up the language model and the dit + vae with:

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


These two will run back to back as sub processes. The num samples will trigger a loop (or multi-processing) to generate all the samples. 