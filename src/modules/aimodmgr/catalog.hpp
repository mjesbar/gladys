// Model catalogs - what Gladys can install. Keys are the names models install as: a GGUF file for
// LLM, a folder for ASR. The ASR folder holds the files listed in its AsrFiles.

#ifndef AIMODMGR_CATALOG_HPP
#define AIMODMGR_CATALOG_HPP

#include <QString>
#include <map>

enum class ModelKind { Llm, Asr };
enum class AsrEngine { Transducer, Whisper, SenseVoice };

struct AsrFiles {  // file names inside the ASR folder; unused ones stay empty
  AsrEngine engine;
  QString tokens;
  QString encoder;  // transducer, whisper
  QString decoder;  // transducer, whisper
  QString joiner;   // transducer
  QString model;    // sense voice
};

struct Model {
  QString name;     // label in Settings
  QString url;      // GGUF file, or sherpa-onnx .tar.bz2 archive
  QString sha256;   // lowercase hex of the download
  bool is_default;  // runs when no installed model is selected
  AsrFiles files;   // ASR only
};

inline const std::map<QString, Model> LLM_CATALOG = {
  {"Qwen3.5-0.8B-Q8_0.gguf",
    {"Qwen 3.5 0.8B Instruct",
      "https://huggingface.co/unsloth/Qwen3.5-0.8B-GGUF/resolve/main/Qwen3.5-0.8B-Q8_0.gguf",
      "0ad885ffd4bb022fc4f0d33a3308fa108ef8613159d3b3a67e23abca056b7a6c",
      true}},
  {"qwen2.5-0.5b-instruct-q8_0.gguf",
    {"Qwen 2.5 0.5B Instruct",
      "https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/"
      "qwen2.5-0.5b-instruct-q8_0.gguf",
      "ca59ca7f13d0e15a8cfa77bd17e65d24f6844b554a7b6c12e07a5f89ff76844e",
      false}},
  {"Llama-3.2-1B-Instruct-Q4_K_M.gguf",
    {"Llama 3.2 1B Instruct",
      "https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/"
      "Llama-3.2-1B-Instruct-Q4_K_M.gguf",
      "6f85a640a97cf2bf5b8e764087b1e83da0fdb51d7c9fab7d0fece9385611df83",
      false}},
  {"granite-3.1-1b-a400m-instruct-Q5_K_M.gguf",
    {"Granite 3.1 1B-A400M Instruct",
      "https://huggingface.co/bartowski/granite-3.1-1b-a400m-instruct-GGUF/resolve/main/"
      "granite-3.1-1b-a400m-instruct-Q5_K_M.gguf",
      "56d4c5eb69c5131d6868526ecea1a69ed11814729c3eeed390c4623c4ddd137e",
      false}},
  {"Falcon3-1B-Instruct-Q4_K_M.gguf",
    {"Falcon3 1B Instruct",
      "https://huggingface.co/bartowski/Falcon3-1B-Instruct-GGUF/resolve/main/"
      "Falcon3-1B-Instruct-Q4_K_M.gguf",
      "1c92013dac1ab6e703e787f3e0829ca03cc95311e4c113a77950d15ff6dea7b3",
      false}},
};

// Fields: engine, tokens, encoder, decoder, joiner, model.
inline const std::map<QString, Model> ASR_CATALOG = {
  {"sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8",
    {"Parakeet TDT 0.6B V3",
      "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
      "sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2",
      "1495e91b59d7fc53ddca91f3431758ec99f8fa253fdb0e1e851be0f72b14b0f9",
      true,
      {AsrEngine::Transducer,
        "tokens.txt",
        "encoder.int8.onnx",
        "decoder.int8.onnx",
        "joiner.int8.onnx",
        ""}}},
  {"sherpa-onnx-whisper-medium",
    {"Whisper Medium",
      "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
      "sherpa-onnx-whisper-medium.tar.bz2",
      "614b1172557049069d846c29d9399640bce83a4dd6c580decebd9ce2a4f32c33",
      false,
      {AsrEngine::Whisper,
        "medium-tokens.txt",
        "medium-encoder.int8.onnx",
        "medium-decoder.int8.onnx",
        "",
        ""}}},
  {"sherpa-onnx-sense-voice-zh-en-ja-ko-yue-int8-2024-07-17",
    {"SenseVoice Yue 2024-07-17 int8",
      "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
      "sherpa-onnx-sense-voice-zh-en-ja-ko-yue-int8-2024-07-17.tar.bz2",
      "7d1efa2138a65b0b488df37f8b89e3d91a60676e416f515b952358d83dfd347e",
      false,
      {AsrEngine::SenseVoice, "tokens.txt", "", "", "", "model.int8.onnx"}}},
  {"sherpa-onnx-nemo-fast-conformer-transducer-be-de-en-es-fr-hr-it-pl-ru-uk-20k",
    {"NeMo Fast Conformer 20K",
      "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
      "sherpa-onnx-nemo-fast-conformer-transducer-be-de-en-es-fr-hr-it-pl-ru-uk-20k.tar.bz2",
      "8d513b0b0b8b3b668e8801614f5c64d9283f4bd1eb52fa36f8aed80a25ad22a1",
      false,
      {AsrEngine::Transducer, "tokens.txt", "encoder.onnx", "decoder.onnx", "joiner.onnx", ""}}},
};

#endif  // AIMODMGR_CATALOG_HPP
