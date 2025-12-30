import torch
from torch import nn

class TextEncoderWrapper(nn.Module):
    def __init__(self, text_encoder):
        super().__init__()
        self.text_encoder = text_encoder

    def forward(self, input_ids, attention_mask, cfg=None):
        last_hidden_state = self.text_encoder(input_ids=input_ids, attention_mask=attention_mask).last_hidden_state

        if cfg is not None:
            cfg_tensor = cfg.unsqueeze(0)  # Convert to tensor for ONNX
            condition = (cfg_tensor > 1).float()  # Create a condition tensor
            if condition: # This enforces the addition of cfg as a variable
                last_hidden_state = torch.concatenate([condition * last_hidden_state, torch.zeros_like(last_hidden_state)], dim=0)
                res_attention_mask = torch.concatenate([condition * attention_mask, torch.zeros_like(attention_mask)], dim=0)
        else:
            res_attention_mask = attention_mask

        return last_hidden_state, res_attention_mask

class PreLoop(nn.Module):
    def __init__(self, num_codebooks=8, audio_channels=2):
        super().__init__()
        self.num_codebooks = num_codebooks
        self.audio_channels = audio_channels

    def triu_onnx(self, x, diagonal=0):
        '''Taken from: https://github.com/fxmarty/optimum/blob/9482591505517da087f3dff180d59abeffec1655/optimum/exporters/onnx/model_patcher.py#L801'''
        l, w = x.shape
        arange_rows = torch.arange(l, device=x.device)

        arange_cols = torch.arange(w, device=x.device)
        mask = arange_cols.expand(l, w)

        arange_rows = arange_rows[:, None] + diagonal
        mask = mask >= arange_rows
        return x.masked_fill(mask == 0, 0)

    @staticmethod
    def build_delay_pattern_mask(self, input_ids, pad_token_id, max_length):
        input_ids = input_ids.reshape(-1, self.num_codebooks, input_ids.shape[-1])
        bsz, num_codebooks, seq_len = input_ids.shape

        max_length = max_length.unsqueeze(0)  # Convert to tensor for ONNX
        condition = (max_length > 0).long()  # Create a condition tensor

        input_ids_shifted = condition * (
            torch.ones((bsz, num_codebooks, max_length.item()), dtype=torch.long, device=input_ids.device) * -1
        )

        max_length = max_length.item()

        channel_codebooks = num_codebooks // 2 if self.audio_channels == 2 else num_codebooks
        # we only apply the mask if we have a large enough seq len - otherwise we return as is
        if max_length < 2 * channel_codebooks - 1:
            return input_ids.reshape(bsz * num_codebooks, -1), input_ids_shifted.reshape(bsz * num_codebooks, -1)

        # fill the shifted ids with the prompt entries, offset by the codebook idx
        for codebook in range(channel_codebooks):
            if self.audio_channels == 1:
                # mono channel - loop over the codebooks one-by-one
                input_ids_shifted[:, codebook, codebook : seq_len + codebook] = input_ids[:, codebook]
            else:
                # left/right channels are interleaved in the generated codebooks, so handle one then the other
                input_ids_shifted[:, 2 * codebook, codebook : seq_len + codebook] = input_ids[:, 2 * codebook]
                input_ids_shifted[:, 2 * codebook + 1, codebook : seq_len + codebook] = input_ids[:, 2 * codebook + 1]
        # construct a pattern mask that indicates the positions of padding tokens for each codebook
        # first fill the upper triangular part (the EOS padding)
        delay_pattern = self.triu_onnx(
            torch.ones((channel_codebooks, max_length), dtype=torch.int32), diagonal=max_length - channel_codebooks + 1
        ).to(torch.int64)
        # then fill the lower triangular part (the BOS padding)
        delay_pattern = delay_pattern + torch.tril(torch.ones((channel_codebooks, max_length), dtype=torch.int64))

        if self.audio_channels == 2:
            # for left/right channel we need to duplicate every row of the pattern mask in an interleaved fashion
            delay_pattern = delay_pattern.repeat_interleave(2, dim=0)

        delay_pattern = delay_pattern.to(torch.bool)

        mask = ~delay_pattern.to(input_ids.device)
        input_ids = mask * input_ids_shifted + ~mask * pad_token_id

        # find the first position to start generating - this is the first place we have the -1 token
        # and will always be in the first codebook (since it has no codebook offset)
        first_codebook_ids = input_ids[:, 0, :]
        start_ids = (first_codebook_ids == -1).nonzero()[:, 1]
        if len(start_ids) > 0:
            first_start_id = min(start_ids)
        else:
            # we have no tokens that need to be filled - return entire matrix of input ids
            first_start_id = seq_len

        # (bsz * num_codebooks, seq_len) -> (bsz, num_codebooks, seq_len)
        pattern_mask = input_ids.reshape(bsz * num_codebooks, -1)
        input_ids = input_ids[..., :first_start_id].reshape(bsz * num_codebooks, -1)
        
        return input_ids, pattern_mask 

    def forward(self, batch_size: torch.Tensor, decoder_input_ids=None, decoder_attention_mask=None, max_length=torch.tensor(256, dtype=torch.int64)):
        # TODO: Impl for audio input (uses decoder_input_ids and decoder_attention_mask)
        # Equal to #5 _prepare_decoder_input_ids_for_generation
        decoder_start_token_id = 2048
        pad_token_id = 2048
        decoder_input_ids_start = (
            torch.ones((batch_size * self.num_codebooks, 1), dtype=torch.long) * decoder_start_token_id
        )

        if decoder_input_ids is None:
            decoder_input_ids = decoder_input_ids_start
        elif (decoder_input_ids[..., 0] != decoder_start_token_id).all().item():
            decoder_input_ids = torch.cat([decoder_input_ids_start, decoder_input_ids], dim=-1)
            if decoder_attention_mask is not None:
                decoder_attention_mask = decoder_attention_mask
                decoder_attention_mask = torch.cat(
                    (torch.ones_like(decoder_attention_mask)[:, :1], decoder_attention_mask),
                    dim=-1,
                )
                decoder_attention_mask = decoder_attention_mask

        # Build delay pattern mask
        decoder_input_ids, decoder_delay_pattern_mask = self.build_delay_pattern_mask(self=self, input_ids=decoder_input_ids, pad_token_id=pad_token_id, max_length=max_length)
        
        return decoder_input_ids, decoder_delay_pattern_mask
    
class Sample(nn.Module):
    def __init__(self, decoder, enc_proj):
        super().__init__()
        self.decoder = decoder
        self.enc_proj = enc_proj
        self.filter_value = -float('inf')
        self.past_key_values = None

    @staticmethod
    def apply_delay_pattern_mask(input_ids, decoder_pad_token_mask):
        """Apply a delay pattern mask to the decoder input ids, only preserving predictions where
        the mask is set to -1, and otherwise setting to the value detailed in the mask."""
        seq_len = input_ids.shape[-1]
        decoder_pad_token_mask = decoder_pad_token_mask[..., :seq_len]
        input_ids = torch.where(decoder_pad_token_mask == -1, input_ids, decoder_pad_token_mask)
        return input_ids
    
    def logits_process(self, next_token_logits, cfg):
        # ClassifierFreeGuidanceLogitsProcessor
        unguided_bsz = next_token_logits.shape[0] // 2
        cond_logits, uncond_logits = next_token_logits.split(unguided_bsz, dim=0)
        next_token_scores = uncond_logits + (cond_logits - uncond_logits) * cfg
        return next_token_scores

    def logits_warp(self, scores: torch.Tensor, temperature: torch.Tensor, topk: torch.Tensor, topp: torch.Tensor):
        # Temperature
        if temperature != 1.0:
            scores_processed = scores / temperature
        else:
            scores_processed = scores

        # Topk
        top_k = min(topk, scores_processed.size(-1))  # Safety check
        # Remove all tokens with a probability less than the last token of the top-k
        indices_to_remove = scores_processed < torch.topk(scores_processed, top_k)[0][..., -1, None]
        scores_processed = scores_processed.masked_fill(indices_to_remove, self.filter_value)

        # Topp
        if topp < 1.0:
            sorted_logits, sorted_indices = torch.sort(scores_processed, descending=False)
            cumulative_probs = sorted_logits.softmax(dim=-1).cumsum(dim=-1)

            # Remove tokens with cumulative top_p above the threshold (token with 0 are kept)
            sorted_indices_to_remove = cumulative_probs <= (1 - topp)
            # Keep at least min_tokens_to_keep
            sorted_indices_to_remove[..., -1 :] = 0

            # scatter sorted tensors to original indexing
            indices_to_remove = sorted_indices_to_remove.scatter(1, sorted_indices, sorted_indices_to_remove)
            scores_processed = scores_processed.masked_fill(indices_to_remove, self.filter_value)
        return scores_processed

    def forward(
            self, 
            decoder_input_ids, 
            attention_mask, # <- Is encoder attention_mask
            encoder_hidden_states, 
            cfg = torch.tensor(3),
            temperature = torch.tensor(0.7), 
            topk = torch.tensor(500), 
            topp = torch.tensor(0.0)
        ):

        # Input prep
        if cfg is not None:
            cfg_tensor = cfg.unsqueeze(0)  # Convert to tensor for ONNX
            condition = (cfg_tensor > 1).int()  # Create a condition tensor
            if condition:
                model_inputs = (condition * decoder_input_ids).repeat((2,1))
                # if attention_mask is not None: # This is only for decoder_attention_mask (gets when given audio)
                #     model_input_attention_mask = (condition * attention_mask).repeat((2,1))
        
        # if past_key_values is not None:
        #     past_length = past_key_values[0][0].shape[2]
            
        #     if model_inputs.shape[1] > past_length:
        #         remove_prefix_length = past_length
        #     else:
        #         # Default to old behavior: keep only final ID
        #         remove_prefix_length = model_inputs.shape[1] - 1

        #     model_inputs = model_inputs[:, remove_prefix_length:]

            # model_inputs = model_inputs[:, -1:]

        # Forward Loop
        encoder_hidden_states = self.enc_proj(encoder_hidden_states)
        encoder_hidden_states = encoder_hidden_states * attention_mask[..., None]

        outputs = self.decoder(
            input_ids = model_inputs,
            attention_mask = None,
            encoder_hidden_states = encoder_hidden_states,
            encoder_attention_mask = attention_mask,
            inputs_embeds = None,
            output_attentions = False,
            output_hidden_states = False,
            use_cache = False,
            past_key_values = None,
            return_dict = True,
            labels = None,
            head_mask = None,
        )

        # past_key_values = outputs.past_key_values

        next_token_logits = outputs.logits[:, -1, :]

        # CFG processing if cfg is large enough, aka logits_processlist
        if condition:
            next_token_scores = self.logits_process(next_token_logits, cfg)
        else:
            next_token_scores = next_token_logits
        return next_token_scores, encoder_hidden_states

        next_token_scores = self.logits_warp(next_token_scores, temperature, topk, topp)

        probs = nn.functional.softmax(next_token_scores, dim=-1)
        next_tokens = torch.multinomial(probs, num_samples=1).squeeze(1)

        # next_tokens = torch.argmax(next_token_scores, dim=-1) # Could be added for no sample generation

        decoder_input_ids = torch.cat([decoder_input_ids, next_tokens[:, None]], dim=-1)

        return decoder_input_ids, encoder_hidden_states

class DecodeAudioWrapper(nn.Module):
    def __init__(self, audio_encoder):
        super().__init__()
        self.audio_encoder = audio_encoder

    def apply_delay_pattern_mask(self, input_ids, decoder_pad_token_mask):
        seq_len = input_ids.shape[-1]
        decoder_pad_token_mask = decoder_pad_token_mask[..., :seq_len]
        input_ids = torch.where(decoder_pad_token_mask == -1, input_ids, decoder_pad_token_mask)
        return input_ids

    def forward(self, output_ids: torch.Tensor):
        '''Taken from last section of the model'''

        batch_size = 1 # We will only allow sampling of single samples for now, otherwise it might be too slow

        # apply the pattern mask to the final ids
        # output_ids = self.apply_delay_pattern_mask(output_ids, decoder_delay_pattern_mask)

        # revert the pattern delay mask by filtering the pad token id
        # output_ids = output_ids[output_ids != pad_token_id].reshape(
        #     batch_size, 8, -1
        # )

        # append the frame dimension back to the audio codes
        # output_ids = output_ids[None, ...]

        audio_scales = [None] * batch_size

        codec_outputs_left = self.audio_encoder.decode(output_ids[:, :, ::2, :], audio_scales=audio_scales)
        output_values_left = codec_outputs_left.audio_values

        codec_outputs_right = self.audio_encoder.decode(output_ids[:, :, 1::2, :], audio_scales=audio_scales)
        output_values_right = codec_outputs_right.audio_values

        output_values = torch.cat([output_values_left, output_values_right], dim=1)

        return output_values