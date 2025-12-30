import { AutoTokenizer, MusicgenForConditionalGeneration } from '@huggingface/transformers';

// Load tokenizer and model
console.log("Downloading_tokenizer");
const tokenizer = await AutoTokenizer.from_pretrained('Xenova/musicgen-small');
console.log("Downloading_model");
const model = await MusicgenForConditionalGeneration.from_pretrained(
  'Xenova/musicgen-small', { dtype: 'fp32' }
);

// Prepare text input
console.log("prep_ins");
const prompt = '80s pop track with bassy drums and synth';
const inputs = tokenizer(prompt);

// Generate audio
console.log("generate");
const audio_values = await model.generate({
  ...inputs,
  max_new_tokens: 512,
  do_sample: true,
  guidance_scale: 3,
});

// (Optional) Write the output to a WAV file
import wavefile from 'wavefile';
import fs from 'fs';

console.log("writing");
const wav = new wavefile.WaveFile();
wav.fromScratch(1, model.config.audio_encoder.sampling_rate, '32f', audio_values.data);
fs.writeFileSync('musicgen_out.wav', wav.toBuffer());