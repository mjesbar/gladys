#ifndef STT_H
#define STT_H

#include <string>
#include "miniaudio.h"
#include "sherpa-onnx/c-api/c-api.h" // Placeholder, will be resolved by Makefile

class SST {
public:
    // Delete copy constructor and assignment operator for singleton
    SST(const SST&) = delete;
    SST& operator=(const SST&) = delete;

    static bool load(const std::string& model_path);
    static void start();
    static void stop();

private:
    SST() = default; // Private constructor to enforce singleton
    ~SST(); // Private destructor for cleanup

    static SST* m_instance;
    static ma_device m_audio_device;
    static ma_device_config m_audio_config;
    static ma_context m_audio_context;
    static bool m_is_audio_context_initialized;

    static const SherpaOnnxOnlineRecognizer* m_recognizer;
    static const SherpaOnnxOnlineStream* m_stream;

    static std::string m_encoder_path;
    static std::string m_decoder_path;
    static std::string m_joiner_path;
    static std::string m_tokens_path;

    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif // STT_H
