
from transformers import AutoProcessor, MusicgenForConditionalGeneration
import torch, os, glob, json, math, scipy, fire
from torch import nn
import numpy as np
import onnxruntime as ort
import inspect
from models import *
from exporters import *

def softmax(x, axis=-1):
    exp_x = np.exp(x - np.max(x, axis=axis, keepdims=True))  # Subtract max for numerical stability
    return exp_x / np.sum(exp_x, axis=axis, keepdims=True)

def multinomial(probs, num_samples=1):
    # Create an empty array to hold sampled tokens
    batch_size, seq_len, vocab_size = probs.shape
    sampled_tokens = np.zeros((batch_size, seq_len), dtype=np.int32)
    
    for i in range(batch_size):
        for j in range(seq_len):
            # Use np.random.choice to sample from the vocabulary based on probabilities
            sampled_tokens[i, j] = np.random.multinomial(p=probs[i, j, :]).squeeze()
    
    return sampled_tokens

def logits_warp(logits, temperature, top_k, top_p):
    if temperature > 0.0:
        logits = logits / temperature

    # Top K sampling
    top_k_indices = np.argsort(logits)[:,:,-top_k:]
    top_k_probs = np.take_along_axis(logits, top_k_indices, axis=-1)

    # Softmax
    top_k_probs = softmax(top_k_probs, axis=-1)

    # Top
    sorted_indices = np.argsort(top_k_probs, axis=-1)[..., ::-1]
    sorted_probs = np.take_along_axis(top_k_probs, sorted_indices, axis=-1)

    # Top p 
    if top_p < 1.0:
        sorted_indices = np.argsort(top_k_probs, axis=-1)[..., ::-1]
        sorted_probs = np.take_along_axis(top_k_probs, sorted_indices, axis=-1)

        cumulative_probs = np.cumsum(sorted_probs, axis=-1)

        sorted_indices_to_keep = cumulative_probs <= (1 - top_p)
        sorted_indices_to_keep[..., 0] = True
        filtered_probs = np.where(sorted_indices_to_keep, sorted_probs, 0)
        filtered_probs = softmax(filtered_probs, axis=-1) # Chaning this to a different sampling could fix my life.
        top_k_probs = filtered_probs

    # Sample
    return np.array([[top_k_indices[i, : , sorted_indices[i, :, np.random.multinomial(1, codebook, size=(1,)).argmax()]]] for i, codebook in enumerate(top_k_probs.squeeze())]).squeeze()[:, None]

def apply_mask(ids, decoder_pattern_mask):
    seq_len = ids.shape[-1]
    decoder_pattern_mask = decoder_pattern_mask[..., :seq_len]
    return np.where(decoder_pattern_mask == -1, ids, decoder_pattern_mask)

def test_onnx(
        folder: str, 
        inputs: dict,
        max_len: int, 
        cfg: int, 
        temperature: float,
        top_k: int, 
        top_p: float, 
        sampling_rate: int
    ):
    ort_session = ort.InferenceSession(f"{folder}/text_encoder.onnx")
    ort_session.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL

    input_ids_np = inputs['input_ids'].detach().numpy()
    attention_mask_np = inputs['attention_mask'].detach().numpy()

    # Run the model
    ort_inputs = {
        'input_ids': input_ids_np,
        'attention_mask': attention_mask_np,
        'cfg': np.array([cfg], dtype=np.int64)
    }
    encoded, attention_mask = ort_session.run(None, ort_inputs)

    ort_session = ort.InferenceSession(f"{folder}/pre_loop.onnx")
    ort_session.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL

    batch_size = torch.tensor(1, dtype=torch.int64).detach().numpy()
    max_length = torch.tensor(max_len, dtype=torch.int64).detach().numpy()

    # Run the model
    ort_inputs = {
        'batch_size': batch_size,
        'max_length': max_length
    }
    decoder_input_ids, decoder_delay_pattern_mask = ort_session.run(None, ort_inputs)

    ort_session = ort.InferenceSession(f"{folder}/sampler.onnx")
    ort_session.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL

    ort_inputs = {
        'decoder_input_ids': decoder_input_ids, 
        'attention_mask': attention_mask.astype(np.int64), 
        'encoder_hidden_states.1': encoded, 
        'cfg': np.array([cfg], dtype=np.int64), 
        # 'temperature': np.array([temperature], dtype=np.float32), 
        # 'topk': np.array([top_k], dtype=np.int64), 
        # 'topp': np.array([top_p], dtype=np.float32)
    }

    for i in range(max_len-1):
        print(i, end='\r')
        ort_inputs['decoder_input_ids'] = apply_mask(ort_inputs['decoder_input_ids'], decoder_delay_pattern_mask)
        # Run the model
        logits, _ = ort_session.run(None, ort_inputs)
        logits = logits[:, None]
        outputs = logits_warp(logits, temperature, top_k, top_p)
        ort_inputs['decoder_input_ids'] = np.concatenate((ort_inputs['decoder_input_ids'], outputs), axis=-1)

    output_ids = apply_mask(ort_inputs['decoder_input_ids'], decoder_delay_pattern_mask)
    output_ids = output_ids[output_ids != 2048].reshape(
        1, 8, -1
    )

    # append the frame dimension back to the audio codes
    output_ids = output_ids[None, ...]

    ort_session = ort.InferenceSession(f"{folder}/audio_token_decoder.onnx")
    ort_session.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL

    # Run the model
    ort_inputs = {
        'output_ids': output_ids # We either need to remove the first tokens or add tokens to the decoder delay_pattern mask, check og for inspo
    }

    output_values = ort_session.run(None, ort_inputs)[0]

    scipy.io.wavfile.write("onnx_res.wav", rate=sampling_rate, data=output_values[0].T)

def export_onnx(
        folder,
        model: MusicgenForConditionalGeneration,
        processor: AutoProcessor,
        text_encoder_wrapper: TextEncoderWrapper, 
        pre_loop: PreLoop, 
        sample: Sample, 
        audio_decoder_wrapper: DecodeAudioWrapper
    ):
    export_configs(processor, model, folder)
    export_text_encoder(text_encoder_wrapper, folder)
    export_preloop(pre_loop, folder)
    export_sampler(sample, folder)
    export_audio_decoder(audio_decoder_wrapper, folder)

def test_torch(
        inputs: dict, 
        text_encoder_wrapper: TextEncoderWrapper, 
        pre_loop: PreLoop, 
        sample: Sample, 
        audio_decoder_wrapper: DecodeAudioWrapper, 
        max_len: int, 
        cfg: int, 
        temperature: float,
        top_k: int, 
        top_p: float, 
        sampling_rate: int
    ):
    with torch.no_grad():
        encoded, attention_mask = text_encoder_wrapper(inputs['input_ids'], inputs['attention_mask'], torch.tensor([cfg], dtype=torch.int64))

        decoder_input_ids, decoder_delay_pattern_mask = pre_loop(torch.tensor(1, dtype=torch.int64), None, None, torch.tensor(max_len, dtype=torch.int64))

        for i in range(max_len-1):
            print(i, end='\r')
        # for i in range(3):
            decoder_input_ids, _ = sample(
                decoder_input_ids, 
                attention_mask, 
                encoded, 
                decoder_delay_pattern_mask, 
                torch.tensor([cfg], dtype=torch.int64), 
                torch.tensor([temperature], dtype=torch.float32), 
                torch.tensor([top_k], dtype=torch.int64), 
                torch.tensor([top_p], dtype=torch.float32)
            )

        output_values = audio_decoder_wrapper(decoder_input_ids, decoder_delay_pattern_mask, torch.tensor([2048], dtype=torch.int64))

    scipy.io.wavfile.write("torch_output.wav", rate=sampling_rate, data=output_values[0].detach().numpy().T)

def main(
        export_onnx_ = True,
        test_onnx_ = True,
        test_torch_script = False,
        do_model_sample = False,
        sampling_rate = 32000,
        cfg = 3,
        temperature = 0.7,
        top_k = 250,
        top_p = 1.0,
        max_len = 256,
        folder = './musicgen-stereo-small'
    ):
    os.makedirs(folder, exist_ok=True)

    processor = AutoProcessor.from_pretrained("facebook/musicgen-stereo-small")

    if do_model_sample or export_onnx or test_torch_script:
        model = MusicgenForConditionalGeneration.from_pretrained("facebook/musicgen-stereo-small")
        model.eval()

    inputs = processor(
        text=["80s pop track with bassy drums and synth asd asd asd"],
        padding=True,
        return_tensors="pt",
    )
    if do_model_sample:
        res = model.generate(**inputs, do_sample=True, guidance_scale=cfg, max_new_tokens=max_len, temperature=temperature, top_k=top_k, top_p=top_p)

        scipy.io.wavfile.write("org_output.wav", rate=sampling_rate, data=res[0].detach().numpy().T)

    # Init the classes
    text_encoder_wrapper = TextEncoderWrapper(model.text_encoder)
    pre_loop = PreLoop(model.config.decoder.num_codebooks, model.config.decoder.audio_channels)
    sample = Sample(model.decoder, model.enc_to_dec_proj)
    audio_decoder_wrapper = DecodeAudioWrapper(model.audio_encoder)

    if test_torch_script:
        test_torch(
            inputs,
            text_encoder_wrapper,
            pre_loop,
            sample,
            audio_decoder_wrapper,
            max_len,
            cfg,
            temperature,
            top_k,
            top_p,
            sampling_rate
        )

    if export_onnx_:
        export_onnx(
            folder,
            model,
            processor,
            text_encoder_wrapper,
            pre_loop,
            sample,
            audio_decoder_wrapper
        )

    if test_onnx_:
        test_onnx(
            folder,
            inputs,
            max_len,
            cfg,
            temperature,
            top_k,
            top_p,
            sampling_rate
        )

if __name__ == "__main__":
    fire.Fire(main)