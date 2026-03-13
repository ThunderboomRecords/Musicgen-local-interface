# Download ACE-Step GGUF models from HuggingFace
# Usage: powershell -ExecutionPolicy Bypass -File download-models.ps1

$Repo = "Serveurperso/ACE-Step-1.5-GGUF"
$BaseUrl = "https://huggingface.co/$Repo/resolve/main"
$ModelsDir = "$env:LOCALAPPDATA\MusicGenVST\models"

New-Item -ItemType Directory -Force -Path $ModelsDir | Out-Null

$Models = @(
    "vae-BF16.gguf",
    "Qwen3-Embedding-0.6B-Q8_0.gguf",
    "acestep-5Hz-lm-4B-Q8_0.gguf",
    "acestep-v15-turbo-Q8_0.gguf",
    "acestep-v15-sft-Q8_0.gguf"
)

foreach ($Model in $Models) {
    $OutPath = Join-Path $ModelsDir $Model
    if (Test-Path $OutPath) {
        Write-Host "[OK] $Model"
        continue
    }
    Write-Host "[Download] $Model"
    try {
        Invoke-WebRequest -Uri "$BaseUrl/$Model" -OutFile $OutPath -UseBasicParsing
    } catch {
        Write-Host "[Error] Failed to download $Model : $_"
    }
}

Write-Host "[Done] Models ready in $ModelsDir"
