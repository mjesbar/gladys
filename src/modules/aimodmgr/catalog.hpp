// Model catalog - the llama.cpp GGUF and sherpa-onnx models Gladys can install. The default of each
// kind is in use.

#ifndef AIMODMGR_CATALOG_HPP
#define AIMODMGR_CATALOG_HPP

#include <QString>
#include <vector>

enum class ModelKind { Llm, Asr };

struct ModelInfo {
  QString name;  // label in Settings
  ModelKind kind;
  QString file;     // GGUF file name, or sherpa-onnx folder name (archive without .tar.bz2)
  QString url;      // GGUF file, or sherpa-onnx .tar.bz2 archive
  QString sha256;   // lowercase hex of the download
  bool is_default;  // the model Gladys runs with; it cannot be uninstalled
};

inline const std::vector<ModelInfo> MODEL_CATALOG = {
  {"Qwen 3.5 0.8B Instruct",
    ModelKind::Llm,
    "Qwen3.5-0.8B-Q8_0.gguf",
    "https://huggingface.co/unsloth/Qwen3.5-0.8B-GGUF/resolve/main/Qwen3.5-0.8B-Q8_0.gguf",
    "0ad885ffd4bb022fc4f0d33a3308fa108ef8613159d3b3a67e23abca056b7a6c",
    true},
  {"Qwen 2.5 0.5B Instruct",
    ModelKind::Llm,
    "qwen2.5-0.5b-instruct-q8_0.gguf",
    "https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/"
    "qwen2.5-0.5b-instruct-q8_0.gguf",
    "ca59ca7f13d0e15a8cfa77bd17e65d24f6844b554a7b6c12e07a5f89ff76844e",
    false},
  {"Llama 3.2 1B Instruct",
    ModelKind::Llm,
    "Llama-3.2-1B-Instruct-Q4_K_M.gguf",
    "https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/"
    "Llama-3.2-1B-Instruct-Q4_K_M.gguf",
    "6f85a640a97cf2bf5b8e764087b1e83da0fdb51d7c9fab7d0fece9385611df83",
    false},
  {"Granite 3.1 1B-A400M Instruct",
    ModelKind::Llm,
    "granite-3.1-1b-a400m-instruct-Q5_K_M.gguf",
    "https://huggingface.co/bartowski/granite-3.1-1b-a400m-instruct-GGUF/resolve/main/"
    "granite-3.1-1b-a400m-instruct-Q5_K_M.gguf",
    "56d4c5eb69c5131d6868526ecea1a69ed11814729c3eeed390c4623c4ddd137e",
    false},
  {"Falcon3 1B Instruct",
    ModelKind::Llm,
    "Falcon3-1B-Instruct-Q4_K_M.gguf",
    "https://huggingface.co/bartowski/Falcon3-1B-Instruct-GGUF/resolve/main/"
    "Falcon3-1B-Instruct-Q4_K_M.gguf",
    "1c92013dac1ab6e703e787f3e0829ca03cc95311e4c113a77950d15ff6dea7b3",
    false},
  {"Parakeet TDT 0.6B V3",
    ModelKind::Asr,
    "sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8",
    "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
    "sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2",
    "1495e91b59d7fc53ddca91f3431758ec99f8fa253fdb0e1e851be0f72b14b0f9",
    true},
  {"Whisper Medium",
    ModelKind::Asr,
    "sherpa-onnx-whisper-medium",
    "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
    "sherpa-onnx-whisper-medium.tar.bz2",
    "614b1172557049069d846c29d9399640bce83a4dd6c580decebd9ce2a4f32c33",
    false},
  {"SenseVoice Yue 2024-07-17 int8",
    ModelKind::Asr,
    "sherpa-onnx-sense-voice-zh-en-ja-ko-yue-int8-2024-07-17",
    "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
    "sherpa-onnx-sense-voice-zh-en-ja-ko-yue-int8-2024-07-17.tar.bz2",
    "7d1efa2138a65b0b488df37f8b89e3d91a60676e416f515b952358d83dfd347e",
    false},
  {"NeMo Fast Conformer 20K",
    ModelKind::Asr,
    "sherpa-onnx-nemo-fast-conformer-transducer-be-de-en-es-fr-hr-it-pl-ru-uk-20k",
    "https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/"
    "sherpa-onnx-nemo-fast-conformer-transducer-be-de-en-es-fr-hr-it-pl-ru-uk-20k.tar.bz2",
    "8d513b0b0b8b3b668e8801614f5c64d9283f4bd1eb52fa36f8aed80a25ad22a1",
    false},
};

#endif  // AIMODMGR_CATALOG_HPP
