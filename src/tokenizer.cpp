#include "../include/tokenizer.hpp"

#include <iostream>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

Tokenizer::Tokenizer(const std::string& spiece_model_path, const std::string& special_tokens_path) {
    sp.Load(spiece_model_path);  // Load SentencePiece model
    load_special_tokens(special_tokens_path);  // Load special tokens
}

void Tokenizer::load_special_tokens(const std::string& special_tokens_path) {
    std::ifstream special_tokens_file(special_tokens_path);
    json special_tokens_json;
    special_tokens_file >> special_tokens_json;

    // Load special tokens (like pad, eos, unk)
    pad_token = special_tokens_json["pad_token"]["content"].get<std::string>();
    eos_token = special_tokens_json["eos_token"]["content"].get<std::string>();
    unk_token = special_tokens_json["unk_token"]["content"].get<std::string>();

    // Add additional special tokens to the token_to_id and id_to_token maps
    for (const auto& token : special_tokens_json["additional_special_tokens"]) {
        std::string token_str = token.get<std::string>();
        token_to_id[token_str] = sp.PieceToId(token_str);
        id_to_token[sp.PieceToId(token_str)] = token_str;
    }
}

// Encode text into token IDs using SentencePiece
std::vector<int> Tokenizer::encode(const std::string& text) {
    std::vector<int> tokens;
    sp.Encode(text, &tokens);

    // Append EOS token at the end
    tokens.push_back(sp.PieceToId(eos_token));
    return tokens;
}

// Decode token IDs back into text using SentencePiece
std::string Tokenizer::decode(const std::vector<int>& tokens) {
    std::string decoded_text;
    sp.Decode(tokens, &decoded_text);
    return decoded_text;
}