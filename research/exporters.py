import torch
from models import *

def export_text_encoder(text_encoder_wrapper: TextEncoderWrapper, folder: str):
    text_encoder_wrapper.eval()

    # Define the dynamic axes for variable-length input shapes
    dynamic_axes = {
        'input_ids': {0: 'batch_size', 1: 'sequence_length'},  # Allow variable batch size and sequence length
        'attention_mask': {0: 'batch_size', 1: 'sequence_length'},  # Allow variable batch size and sequence length
        'encoded': {0: 'batch_size', 1: 'sequence_length'}  # Output will also have variable batch size and sequence length
    }

    # Example input shapes (with batch size = 2, sequence length = 10)
    dummy_input_ids = torch.randint(0, 100, (1, 18), dtype=torch.int64)
    dummy_attention_mask = torch.randint(0, 100, (1, 18), dtype=torch.int64)
    dummy_cfg = torch.tensor(3, dtype=torch.int64)

    # Export the model to ONNX format
    torch.onnx.export(
        text_encoder_wrapper,
        (dummy_input_ids, dummy_attention_mask, dummy_cfg),
        f"{folder}/text_encoder.onnx",
        input_names=['input_ids', 'attention_mask', 'cfg'],
        output_names=['last_hidden_state', 'res_attention_mask'],
        dynamic_axes=dynamic_axes,
        opset_version=17
    )

def export_preloop(pre_loop: PreLoop, folder: str):
    pre_loop.eval()

    # Define the dynamic axes for variable-length input shapes
    dynamic_axes = {
        # 'decoder_input_ids': {0: 'batch_size', 1: 'sequence_length'},
        # 'decoder_attention_mask': {0: 'batch_size', 1: 'sequence_length'},
        'decoder_input_ids': {0: 'batch_size', 1: 'sequence_length'},
        'decoder_delay_pattern_mask': {0: 'batch_size', 1: 'sequence_length'}
    }

    # Example input shapes (with batch size = 2, sequence length = 10)
    dummy_batch_size = torch.tensor(1, dtype=torch.int64)
    dummy_max_length = torch.tensor(256, dtype=torch.int64)

    # Export the model to ONNX format
    torch.onnx.export(
        pre_loop,
        (dummy_batch_size, None, None, dummy_max_length),
        f"{folder}/pre_loop.onnx",
        input_names=['batch_size', 'max_length'],
        output_names=['decoder_input_ids', 'decoder_delay_pattern_mask'],
        dynamic_axes=dynamic_axes,
        opset_version=17
    )

def export_sampler(sample: Sample, folder: str):
    sample.eval()

    # Define the dynamic axes for variable-length input shapes
    dynamic_axes = {
        'decoder_input_ids': {0: 'batch_size', 1: 'sequence_length'},
        'attention_mask': {0: 'batch_size', 1: 'sequence_length'},
        'encoder_hidden_states': {0: 'batch_size', 1: 'sequence_length'}
    }

    # Example input shapes (with batch size = 2, sequence length = 10)
    dummy_decoder_input_ids = torch.randint(0, 100, (8, 1), dtype=torch.int64)
    dummy_attention_mask = torch.randint(0, 100, (2, 18), dtype=torch.int64)
    dummy_encoder_hidden_states = torch.randn((2, 18, 768), dtype=torch.float32)
    dummy_cfg = torch.tensor(3, dtype=torch.int64)
    dummy_temperature = torch.tensor(0.7, dtype=torch.float32)
    dummy_topk = torch.tensor(500, dtype=torch.int64)
    dummy_topp = torch.tensor(0.0, dtype=torch.float32)

    # Export the model to ONNX format
    torch.onnx.export(
        sample,
        (
            dummy_decoder_input_ids, 
            dummy_attention_mask, 
            dummy_encoder_hidden_states, 
            dummy_cfg, 
            dummy_temperature, 
            dummy_topk, 
            dummy_topp,
        ),
        f"{folder}/sampler.onnx",
        input_names=[
            'decoder_input_ids', 
            'attention_mask', 
            'encoder_hidden_states', 
            'cfg', 
            'temperature', 
            'topk', 
            'topp',
        ],
        output_names=['next_token_logits', 'encoder_hidden_states'],
        dynamic_axes=dynamic_axes,
        opset_version=17
    )


def export_audio_decoder(audio_decoder_wrapper: DecodeAudioWrapper, folder: str):
    audio_decoder_wrapper.eval()

    # Define the dynamic axes for variable-length input shapes
    dynamic_axes = {
        'output_ids': {3: 'sequence_length'}, # Allow variable batch size and sequence length
    }

    # Example input shapes (with batch size = 2, sequence length = 10)
    dummy_output_ids = torch.randint(0, 100, (1, 1, 8, 256), dtype=torch.int64)

    # Export the model to ONNX format
    torch.onnx.export(
        audio_decoder_wrapper,
        (dummy_output_ids,),
        f"{folder}/audio_token_decoder.onnx", 
        input_names=[
            'output_ids',
        ],
        output_names=['output_values'],
        dynamic_axes=dynamic_axes,
        opset_version=17
    )

def export_configs(processor, model, folder: str):
    processor.tokenizer.save_pretrained(f'{folder}')
    processor.save_pretrained(f'{folder}')
    model.config.to_json_file(f'{folder}/config.json')
    model.generation_config.to_json_file(f'{folder}/generation_config.json')