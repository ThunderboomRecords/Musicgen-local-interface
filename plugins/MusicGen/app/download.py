from transformers import AutoProcessor, MusicgenForConditionalGeneration, MusicgenMelodyForConditionalGeneration

device = 'cpu'

processor_small = AutoProcessor.from_pretrained("facebook/musicgen-stereo-small", cache_dir="./.cache")
model_small = MusicgenForConditionalGeneration.from_pretrained("facebook/musicgen-stereo-small", cache_dir="./.cache").to(device)