#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <fstream>
#include <sndfile.h>

#include <core/onnxruntime_cxx_api.h>

#include "./tokenizer.hpp"

using namespace std;

template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {
  // initialize original index locations
  vector<size_t> idx(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  // using stable_sort instead of sort
  // to avoid unnecessary index re-orderings
  // when v contains elements of equal values 
  stable_sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}

// Softmax function
vector<float> softmax(const vector<float>& input) {
    vector<float> output(input.size());

    // Find the maximum value in the input to subtract for numerical stability
    float maxInput = *max_element(input.begin(), input.end());

    // Compute the sum of exponentials
    float sum = 0.0;
    for (float val : input) {
        sum += exp(val - maxInput);
    }

    // Compute softmax
    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = exp(input[i] - maxInput) / sum;
    }

    return output;
}

// Function to simulate a multinomial distribution and return the index of the maximum value
int multinomial_argmax(const vector<float>& probabilities) {
    // Create a random engine
    random_device rd;
    mt19937 gen(rd());

    // Create a discrete distribution based on the probabilities vector
    discrete_distribution<> dist(probabilities.begin(), probabilities.end());

    // Simulate the multinomial draw
    int index = dist(gen);

    // Return the index (argmax)
    return index;
}


//Could be replaced with output in wav fn.
string generate(
    Tokenizer &tokenizer,
    map<string, unique_ptr<Ort::Session>> &sessions,
    vector<string> input_sentences,
    int64_t max_len,
    int64_t pad_token_id,
    float temperature,
    int64_t top_k,
    float top_p
){ 
    cout << "Tokenizing" << endl;
    // There could be a problem wrt the shaping of the tokens
    vector<vector<int64_t>> sample_tokens;
    vector<vector<int64_t>> attention_masks;
    for(auto sentence: input_sentences){
        vector<int> outcome = tokenizer.encode(sentence);
        vector<int64_t> tokenized(outcome.size());
        transform(outcome.begin(), outcome.end(), tokenized.begin(), [](int x) {
            return static_cast<int64_t>(x);
        });
        sample_tokens.push_back(tokenized);
        attention_masks.push_back(vector<int64_t>(tokenized.size(), 1));
    }
    cout << "Done tokenizing" << endl;

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    cout << "Building decoder pattern mask" << endl;

    vector<int64_t> mask_builder_input_ids(4 * 16, -1);

    vector<int64_t> input_ids_shape = {4, 16};
    vector<int64_t> pad_token_id_shape = {1};
    vector<int64_t> max_length_shape = {1};
    
    Ort::Value input_ids_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, mask_builder_input_ids.data(), mask_builder_input_ids.size(), input_ids_shape.data(), input_ids_shape.size());
    Ort::Value pad_token_id_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, &pad_token_id, 1, pad_token_id_shape.data(), pad_token_id_shape.size());
    Ort::Value max_length_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, &max_len, 1, max_length_shape.data(), max_length_shape.size());
 
    array<Ort::Value, 3> mask_builder_input_tensors = {move(input_ids_tensor), move(pad_token_id_tensor), move(max_length_tensor)};
    const char* mask_builder_input_names[] = {"input_ids", "pad_token_id", "max_length"};
    vector<const char*> mask_builder_output_names = {"input_ids_edited", "delay_pattern_mask"};  // Replace with the actual output name
    // Run model
    auto mask_builder_output_tensors = sessions["mask_builder"]->Run(
        Ort::RunOptions{nullptr}, 
        mask_builder_input_names, 
        mask_builder_input_tensors.data(), 
        mask_builder_input_tensors.size(), 
        mask_builder_output_names.data(), 
        mask_builder_output_names.size()
    );

    int64_t* decoder_pattern_mask_table = mask_builder_output_tensors[1].GetTensorMutableData<int64_t>();
    vector<int64_t> decoder_pattern_mask_table_shape = mask_builder_output_tensors[1].GetTensorTypeAndShapeInfo().GetShape();
    vector<int64_t> decoder_pattern_mask(decoder_pattern_mask_table_shape[0] * decoder_pattern_mask_table_shape[1]);

    for(size_t mask_i = 0; mask_i < decoder_pattern_mask.size(); mask_i++) {
        decoder_pattern_mask[mask_i] = decoder_pattern_mask_table[mask_i];
    }

    cout << "Done building decoder pattern mask" << endl;

    vector<vector<vector<float>>> audio_samples;
    for (size_t i = 0; i < sample_tokens.size(); ++i) {
        cout << "Encoding text" << endl;

        // Prepare the inputs
        vector<int64_t> tokens = sample_tokens[i];
        vector<int64_t> attention_mask = attention_masks[i];

        vector<int64_t> tokens_shape = {1, static_cast<int64_t>(sample_tokens[i].size())};
        vector<int64_t> attention_mask_shape = {1, static_cast<int64_t>(attention_masks[i].size())};
        
        Ort::Value input_ids_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, tokens.data(), tokens.size(), tokens_shape.data(), tokens_shape.size());
        Ort::Value attention_mask_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, attention_mask.data(), attention_mask.size(), attention_mask_shape.data(), attention_mask_shape.size());
        
        array<Ort::Value, 2> text_encoder_input_tensors = {move(input_ids_tensor), move(attention_mask_tensor)};
        const char* text_encoder_input_names[] = {"input_ids", "attention_mask"};
        vector<const char*> text_encoder_output_names = {"last_hidden_state"};  // Replace with the actual output name
        // Run model
        auto text_encoder_output_tensors = sessions["text_encoder"]->Run(
            Ort::RunOptions{nullptr}, 
            text_encoder_input_names, 
            text_encoder_input_tensors.data(), 
            text_encoder_input_tensors.size(), 
            text_encoder_output_names.data(), 
            text_encoder_output_names.size()
        );

        float* text_encoder_table = text_encoder_output_tensors[0].GetTensorMutableData<float>();
        vector<int64_t> text_encoder_table_shape = text_encoder_output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
        vector<float> encoder_hidden_states(text_encoder_table_shape[0] * text_encoder_table_shape[1] * text_encoder_table_shape[2]);

        for(size_t mask_i = 0; mask_i < encoder_hidden_states.size(); mask_i++) {
            encoder_hidden_states[mask_i] = text_encoder_table[mask_i];
        }

        cout << "Done encoding text" << endl;

        // Loc these in
        vector<int64_t> encoder_hidden_states_shape = {1, static_cast<int64_t>(tokens.size()), 768};

        cout << "Sampling tokens" << endl;
        vector<int64_t> decoder_tokens(4 * max_len, 0);
        for(size_t codebook_step = 0; codebook_step < 4; codebook_step++){
            decoder_tokens[codebook_step * max_len] = 2048;
        }
        for(int64_t step = 1; step < max_len; step++) {
            cout << "Step: " << step << endl;  // Print step number

            vector<int64_t> decoder_input_ids(4 * step, 2048);
            for(size_t decoder_input_step = 0; decoder_input_step < decoder_input_ids.size(); decoder_input_step++){
                int64_t codebook_step = floor(decoder_input_step / step);
                decoder_input_ids[decoder_input_step] = decoder_tokens[(codebook_step * (max_len-1)) + (decoder_input_step % step)];
            }

            // Apply the decoder pattern_mask
            for(size_t mask_apply_step = 0; mask_apply_step < decoder_input_ids.size(); mask_apply_step++) {
                int64_t codebook_step = floor(mask_apply_step / step);
                if (decoder_pattern_mask[(codebook_step * (max_len-1)) + (mask_apply_step % step)] != -1) {
                    decoder_input_ids[mask_apply_step] = decoder_pattern_mask[(codebook_step * (max_len-1)) + (mask_apply_step % step)];
                }
            }

            // Prepare the inputs
            vector<int64_t> decoder_input_ids_shape = {4, step};
            Ort::Value attention_mask_tensor = Ort::Value::CreateTensor<int64_t>(
                memory_info, 
                attention_mask.data(), 
                attention_mask.size(), 
                attention_mask_shape.data(), 
                attention_mask_shape.size()
            );
            Ort::Value decoder_input_ids_tensor = Ort::Value::CreateTensor<int64_t>(
                memory_info, 
                decoder_input_ids.data(), 
                decoder_input_ids.size(), 
                decoder_input_ids_shape.data(), 
                decoder_input_ids_shape.size()
            );
            Ort::Value encoder_hidden_states_tensor = Ort::Value::CreateTensor<float>(
                memory_info, 
                encoder_hidden_states.data(), 
                encoder_hidden_states.size(), 
                encoder_hidden_states_shape.data(), 
                encoder_hidden_states_shape.size()
            );

            array<Ort::Value, 3> decoder_input_tensors = {
                move(attention_mask_tensor), 
                move(decoder_input_ids_tensor), 
                move(encoder_hidden_states_tensor)
            };
            const char* decoder_input_names[] = {"encoder_attention_mask", "input_ids", "encoder_hidden_states"};
            vector<string> output_names;
            output_names.push_back("logits");
            for (int i = 0; i <= 23; ++i) {
                output_names.push_back("present." + to_string(i) + ".decoder.key");
                output_names.push_back("present." + to_string(i) + ".decoder.value");
                output_names.push_back("present." + to_string(i) + ".encoder.key");
                output_names.push_back("present." + to_string(i) + ".encoder.value");
            }

            vector<const char*> output_names_cstr;
            for (const string& name : output_names) {
                output_names_cstr.push_back(name.c_str());
            }

            auto output_tensors = sessions["text_decoder"]->Run(
                Ort::RunOptions{nullptr}, 
                decoder_input_names, 
                decoder_input_tensors.data(), 
                decoder_input_tensors.size(), 
                output_names_cstr.data(), 
                output_names_cstr.size()
            );

            float* logits_output = output_tensors[0].GetTensorMutableData<float>();
            vector<int64_t> logits_table_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
            size_t codebooks = logits_table_shape[0];
            size_t seq_length = logits_table_shape[1];
            size_t vocab = logits_table_shape[2];

            // cout << logits_table_shape[0] << endl;
            // cout << logits_table_shape[1] << endl;
            // cout << logits_table_shape[2] << endl;

            vector<float> sampled_logits(codebooks);
            vector<vector<vector<float>>> logits_unfolded(codebooks, vector<vector<float>>(seq_length, vector<float>(vocab)));
            for (size_t h = 0; h < codebooks; ++h) {
                for (size_t j = 0; j < seq_length; ++j) {
                    for (size_t k = 0; k < vocab; ++k) {
                        logits_unfolded[h][j][k] = logits_output[h * seq_length * vocab + j * vocab + k];
                    }
                }
            }
            vector<vector<vector<float>>> logits(codebooks, vector<vector<float>>(1, vector<float>(vocab)));
            for (size_t h = 0; h < codebooks; ++h) {
                for (size_t k = 0; k < vocab; ++k) {
                    logits[h][0][k] = logits_unfolded[h][seq_length-1][k];
                }
            }

            if(true){
                // Apply temperature
                if (temperature > 0.0) {
                    for (size_t i = 0; i < codebooks; ++i) {
                        for (size_t k = 0; k < 2048; ++k) {
                            logits[i][0][k] = logits[i][0][k] / temperature;
                        }
                    }
                }

                // Perform top-K sampling
                vector<vector<vector<float>>> top_k_probs(codebooks, vector<vector<float>>(1, vector<float>(top_k)));
                vector<vector<vector<float>>> top_k_indices(codebooks, vector<vector<float>>(1, vector<float>(top_k)));
                for (size_t h = 0; h < codebooks; ++h) {
                    int k = 0;
                    for (auto j: sort_indexes(logits[h][0])) {
                        top_k_probs[h][0][k] = logits[h][0][j];
                        top_k_indices[h][0][k] = j;
                        k++;
                        if(k >= top_k){
                            break;
                        }
                    }
                }
                
                // Do softmax
                for (size_t h = 0; h < codebooks; ++h) {
                    top_k_probs[h][0] = softmax(top_k_probs[h][0]);
                }

                // sampled_logits
                if (top_p < 1.0) {
                    // Perform top-P sampling
                    vector<vector<vector<float>>> sorted_probs(codebooks, vector<vector<float>>(1, vector<float>(top_k)));
                    vector<vector<vector<float>>> sorted_indices(codebooks, vector<vector<float>>(1, vector<float>(top_k)));
                    for (size_t h = 0; h < codebooks; ++h) {
                        vector<size_t> sorted = sort_indexes(top_k_probs[h][0]);
                        reverse(sorted.begin(), sorted.end());
                        int k = 0;
                        for (auto j: sorted) {
                            sorted_probs[h][0][k] = top_k_probs[h][0][j];
                            sorted_indices[h][0][k] = top_k_indices[h][0][j];
                            k++;
                        }
                    }

                    for (size_t h = 0; h < codebooks; ++h) {
                        vector<float> cumsum(top_k, 0.0);
                        vector<float> probs_to_keep(top_k, 0.0);
                        cumsum[0] = sorted_probs[h][0][0];
                        probs_to_keep[0] = sorted_probs[h][0][0];
                        for (size_t k = 1; k < top_k; ++k) {
                            int cumprob = cumsum[k-1] + sorted_probs[h][0][k];
                            probs_to_keep[k] = sorted_probs[h][0][k];
                            cumsum[k] = cumprob;
                            if(cumprob > (1 - top_p)){ // We have all the probs
                                break;
                            }
                        }
                        probs_to_keep = softmax(probs_to_keep);
                        // float max_prob = *max_element(probs_to_keep.begin(), probs_to_keep.end());
                        // if (max_prob == 0.0) {
                        //     probs_to_keep[0] = 1;  // Just take the first prob
                        //     max_prob = 1;
                        // }

                        // // Normalize by dividing each probability by the maximum
                        // vector<float> normalized_probs(top_k, 0);
                        // for (size_t i = 0; i < probs_to_keep.size(); ++i) {
                        //     normalized_probs[i] = probs_to_keep[i] / max_prob;
                        // } 

                        sampled_logits[h] = sorted_indices[h][0][multinomial_argmax(probs_to_keep)];

                    }

                } else {
                    for (size_t h = 0; h < codebooks; ++h) {
                        sampled_logits[h] = top_k_indices[h][0][multinomial_argmax(top_k_probs[h][0])];
                    }
                }
            } else {
                for (size_t h = 0; h < codebooks; ++h) {
                    auto max_it = max_element(logits[h][0].begin(), logits[h][0].end());
                    int argmax = distance(logits[h][0].begin(), max_it);
                    sampled_logits[h] = argmax;
                }

            }

            for(size_t decoder_input_step = 0; decoder_input_step < codebooks; decoder_input_step++){
                decoder_tokens[(decoder_input_step * max_len) + step] = sampled_logits[decoder_input_step];
            }
        } 

        cout << "Done sampling tokens" << endl;

        cout << "Start audio processing" << endl;
        // Apply the decoder pattern_mask
        for(size_t mask_apply_step = 0; mask_apply_step < decoder_tokens.size(); mask_apply_step++) {
            if (decoder_pattern_mask[mask_apply_step] != -1) {
                decoder_tokens[mask_apply_step] = decoder_pattern_mask[mask_apply_step];
            }
        }

        decoder_tokens.erase(remove(decoder_tokens.begin(), decoder_tokens.end(), 2048), decoder_tokens.end());

        ofstream Coutfile("./C_output.txt");

        if (Coutfile.is_open()) {
            // Write each element of the vector to the file, one element per line
            for (const auto& value : decoder_tokens) {
                Coutfile << value << std::endl;  // Each value on a new line
            }
            Coutfile.close();  // Close the file
        } else {
            std::cerr << "Unable to open file" << std::endl;
        }

        // Prepare the inputs
        vector<int64_t> audio_codes_shape = {1, 1, 4, max_len - 4};
        Ort::Value audio_codes_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, decoder_tokens.data(), decoder_tokens.size(), audio_codes_shape.data(), audio_codes_shape.size());
        array<Ort::Value, 1> audio_decoder_input_tensors = {move(audio_codes_tensor)};
        const char* audio_codes_input_names[] = {"audio_codes"};
        vector<const char*> audio_codes_output_names = {"audio_values"};  // Replace with the actual output name
        // Run model
        auto audio_decoder_output_tensors = sessions["audio_decoder"]->Run(
            Ort::RunOptions{nullptr}, 
            audio_codes_input_names, 
            audio_decoder_input_tensors.data(), 
            audio_decoder_input_tensors.size(), 
            audio_codes_output_names.data(), 
            audio_codes_output_names.size()
        );

        float* audio_data_table = audio_decoder_output_tensors[0].GetTensorMutableData<float>();
        vector<int64_t> audio_data_table_shape = audio_decoder_output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
        vector<float> audio_data(audio_data_table_shape[0] * audio_data_table_shape[1] * audio_data_table_shape[2]);

        for(size_t audio_data_i = 0; audio_data_i < audio_data.size(); audio_data_i++) {
            audio_data[audio_data_i] = audio_data_table[audio_data_i];
        }

        cout << audio_data.size() << endl;
        cout << "Done audio processing" << endl;

        SF_INFO sfinfo;
        sfinfo.channels = 1;  // Mono audio
        sfinfo.samplerate = 32000;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

        SNDFILE* outfile = sf_open("./output.wav", SFM_WRITE, &sfinfo);
        sf_write_float(outfile, audio_data.data(), audio_data.size());
        sf_close(outfile);
    }

    return input_sentences[0];
}

int main(){
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    Tokenizer tokenizer("./musicgen-small/spiece.model", "./musicgen-small/special_tokens_map.json");
    cout << "Loaded the tokenizer" << endl;

    map<string, unique_ptr<Ort::Session>> sessions;

    sessions["mask_builder"] = make_unique<Ort::Session>(env, "./musicgen-small/build_delay_pattern_mask.onnx", session_options);
    sessions["text_encoder"] = make_unique<Ort::Session>(env, "./musicgen-small/text_encoder.onnx", session_options);
    sessions["text_decoder"] = make_unique<Ort::Session>(env, "./musicgen-small/decoder_model.onnx", session_options);
    sessions["audio_decoder"] = make_unique<Ort::Session>(env, "./musicgen-small/encodec_decode.onnx", session_options);
    cout << "Model sessions loaded" << endl;

    string test = generate(
        tokenizer,
        sessions,
        vector<string> {"80s pop track with bassy drums and synth"},
        128,
        2048,
        0.7,
        250,
        0.5
    );
}