#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <sentencepiece_processor.h>

class Tokenizer {
public:
    Tokenizer(const std::string& spiece_model_path, const std::string& special_tokens_path);

    std::vector<int> encode(const std::string& text);
    std::string decode(const std::vector<int>& tokens);

private:
    sentencepiece::SentencePieceProcessor sp;
    std::unordered_map<std::string, int> token_to_id;
    std::unordered_map<int, std::string> id_to_token;
    std::string pad_token, eos_token, unk_token;

    void load_special_tokens(const std::string& special_tokens_path);
};

#endif // TOKENIZER_HPP