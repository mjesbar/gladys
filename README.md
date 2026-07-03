# Gladys

**Fully local, cross-platform dictation tool.**  
Speak, and your words appear where you need them — no cloud, no API keys, no AI agent services.

Inspired by tools like Whisper variants and Apple Dictation, Gladys runs entirely on your machine using [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) for speech-to-text and [llama.cpp](https://github.com/ggml-org/llama.cpp) for local transcription formatting. It works on **Windows** and **Linux**, with any GPU vendor (Vulkan support).

---

## Features

- **100% local** — no remote API calls, no data leaves your machine.
- **Direct paste** — transcribed text is copied and pasted directly into the focused text area (chat input, search bars, code editors, etc.).
- **Quick keyboard shortcut** — global hotkey to toggle recording on/off.
- **Microphone source selection** — choose your preferred audio input device.
- **Dictionary / aliases** — define word replacements that are applied during transcription.
- **Style options** — toggle markdown mode, allow accents, allow punctuation.
- **Cross-platform** — Windows and Linux (macOS support in progress).

---

## How it works

1. Press the global hotkey to start recording.
2. Speak — audio is captured via [miniaudio](https://miniaud.io/) and streamed to sherpa-onnx for real-time speech recognition.
3. The raw transcription is sent to a local llama.cpp server for formatting (e.g., adding punctuation, capitalisation).
4. The final text is copied to your clipboard and pasted into the currently focused application via simulated keystrokes.

All processing happens on your local machine. No internet connection is required after the initial model download.

---

## Dependencies

- [Qt 6.8+](https://www.qt.io/) (Core, Gui, Widgets, Network, Svg)
- [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) — speech-to-text inference
- [llama.cpp](https://github.com/ggml-org/llama.cpp) — local LLM server for transcription formatting
- [miniaudio](https://miniaud.io/) — audio capture (single-header, bundled)
- [Vulkan](https://www.vulkan.org/) — GPU acceleration for llama.cpp (optional but recommended)
- X11 development libraries (Linux only)

---

## Build from source

### Prerequisites

- CMake 3.16+
- C++17 compiler
- Qt 6.8+ (install via [aqt](https://github.com/miurahr/aqtinstall) or your package manager)

  ```bash
  # Linux x64
  aqt install-qt linux desktop 6.8.3 linux_gcc_64

  # Windows x64
  aqt install-qt windows desktop 6.8.3 win64_msvc2022_64

  # macOS arm64
  aqt install-qt mac desktop 6.8.3 clang_64
  ```

- X11 development headers (Linux only):

  ```bash
  # Debian/Ubuntu
  sudo apt install libx11-dev
  ```

### Build

```bash
# Clone the repository
git clone https://github.com/mjesbar/gladys.git
cd gladys

# Fetch models (llama.cpp binary, GGUF model, sherpa-onnx model)
make fetch-models

# Build
make build
```

The binary and all runtime dependencies will be placed in the `dist/` directory.

### Run

```bash
make run
```

Or directly:

```bash
./dist/gladys
```

### Package for distribution

```bash
# Set QT_ROOT_DIR to your Qt installation path, then:
make package-linux   # or package-win / package-macos
```

---

## Usage

1. Launch Gladys — it will appear as a system tray icon.
2. Press the global hotkey (default: configurable) to start recording.
3. Speak clearly — the transcription will appear in real-time.
4. Press the hotkey again to stop recording. The final text is pasted automatically.
5. Right-click the tray icon to open Settings, where you can:
   - Select your microphone input source.
   - Add dictionary aliases (e.g., "gladys" → "Gladys").
   - Toggle markdown mode, accents, and punctuation.

---

## License

[GNU General Public License v3.0](LICENSE)

---

## Acknowledgements

- [llama.cpp](https://github.com/ggml-org/llama.cpp) — local LLM inference
- [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) — speech-to-text engine
- [miniaudio](https://miniaud.io/) — audio capture library
- [VulkanKompute](https://github.com/KomputeProject/kompute) — GPU compute framework

---

## Contributing

Contributions, issues, and feature requests are welcome.  
Open an [issue](https://github.com/mjesbar/gladys/issues) or submit a pull request.
