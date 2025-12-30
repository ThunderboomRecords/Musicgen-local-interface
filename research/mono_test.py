from transformers import AutoProcessor, MusicgenForConditionalGeneration
import torch, os, glob, json, math
from torch import nn
import numpy as np
import onnxruntime as ort
import inspect

def softmax(x, axis=-1):
    exp_x = np.exp(x - np.max(x, axis=axis, keepdims=True))  # Subtract max for numerical stability
    return exp_x / np.sum(exp_x, axis=axis, keepdims=True)

# Step 2: Multinomial sampling from probabilities
def multinomial(probs, num_samples=1):
    # Create an empty array to hold sampled tokens
    batch_size, seq_len, vocab_size = probs.shape
    sampled_tokens = np.zeros((batch_size, seq_len), dtype=np.int32)
    
    for i in range(batch_size):
        for j in range(seq_len):
            # Use np.random.choice to sample from the vocabulary based on probabilities
            sampled_tokens[i, j] = np.random.multinomial(p=probs[i, j, :]).squeeze()
    
    return sampled_tokens

def apply_mask(decoder_pattern_mask, ids):
    seq_len = ids.shape[-1]
    decoder_pattern_mask = decoder_pattern_mask[..., :seq_len]
    return np.where(decoder_pattern_mask == -1, ids, decoder_pattern_mask)
    

def main():
    folder = './musicgen-small'
    os.makedirs(folder, exist_ok=True)

    cfg = 3
    temperature = 0.7
    top_k = 250
    top_p = 0.5
    max_len = 256

    # This processor can be ignored in the C++ code. I will figure this out somehow.
    processor = AutoProcessor.from_pretrained("facebook/musicgen-stereo-small")
    inputs = processor(
        text=["80s pop track with bassy drums and synth"],
        padding=True,
        return_tensors="pt",
    )

    ort_session_mask = ort.InferenceSession(f"{folder}/build_delay_pattern_mask.onnx")

    _, decoder_pattern_mask = ort_session_mask.run(None, {
        'input_ids': np.ones((4,16), dtype=np.int64) * -1,
        'pad_token_id': np.array([2048], dtype=np.int64),
        'max_length': np.array([256], dtype=np.int64),
    })

    ort_session = ort.InferenceSession(f"{folder}/text_encoder.onnx")

    input_ids_np = inputs['input_ids'].detach().numpy()
    attention_mask_np = inputs['attention_mask'].detach().numpy()

    # Run the model
    ort_inputs = {
        'input_ids': input_ids_np,
        'attention_mask': attention_mask_np
    }
    encoded = ort_session.run(None, ort_inputs)[0]

    ort_session = ort.InferenceSession(f"{folder}/decoder_model.onnx")

    ort_inputs = {
        'encoder_attention_mask': attention_mask_np, 
        'input_ids': np.ones((4,1), dtype=np.int64) * 2048, 
        'encoder_hidden_states': encoded, 
    }

    for i in range(max_len - 1):
        print(i, end='\r')
        ort_inputs['input_ids'] = apply_mask(decoder_pattern_mask, ort_inputs['input_ids'])
        logits = ort_session.run(None, ort_inputs)[0]
        logits = logits[:, -1, :][:, None]
        # Apply temperature
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
        logits = np.array([[top_k_indices[i, : , sorted_indices[i, :, np.random.multinomial(1, codebook, size=(1,)).argmax()]]] for i, codebook in enumerate(top_k_probs.squeeze())]).squeeze()[:, None]

        # logits = np.argmax(logits, axis=-1)[:, -1]
        ort_inputs['input_ids'] = np.concatenate((ort_inputs['input_ids'], logits), axis=-1)

    output_ids = ort_inputs['input_ids']
    output_ids = apply_mask(decoder_pattern_mask, output_ids)
    output_ids = output_ids[output_ids != 2048].reshape(
        1, 4, -1
    )

    # append the frame dimension back to the audio codes
    output_ids = output_ids[None, ...]

    ort_session = ort.InferenceSession(f"{folder}/encodec_decode.onnx")

    output_values = ort_session.run(None, {'audio_codes': output_ids})[0]

    # Write the wav to file using output_values