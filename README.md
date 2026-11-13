<p align="center">
  <img src="src/assets/icons/icon.svg" alt="Gladys" width="200" height="200">
</p>

<p align="center">
  <img src="src/assets/icons/title.svg" alt="Gladys" height="160">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/license-PolyForm%20Shield%201.0.0-blue" alt="License: PolyForm Shield 1.0.0">
  <img src="https://img.shields.io/badge/Qt-6.8%2B-41CD52" alt="Qt 6.8+">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C" alt="C++17">
  <img src="https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey" alt="Linux | Windows | macOS">
</p>

<p align="center">
<strong>Fully local, cross-platform dictation tool.</strong>  
Speak, and your words appear where you need them — no cloud, no API keys, no AI agent services.
</p>

Gladys runs entirely on your machine using [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) for speech-to-text and [llama.cpp](https://github.com/ggml-org/llama.cpp) for local transcription formatting. It runs on **Windows**, **Linux** and **macOS** (Vulkan GPU support on Windows and Linux, any vendor).

---

# Product

### Features

| Feature             | Description                                      |
| ------------------- | ------------------------------------------------ |
| 100% local          | No remote API calls, no data leaves your machine |
| Paste everywhere    | Text is copied and pasted into the focused input |
| Dictionary/Snippets | Word replacements applied during formatting      |
| Style options       | Markdown mode, allow accents, allow punctuation  |

---

### Architecture

![Architecture](src/assets/Architecture.png)

---

### Usage

1. Launch Gladys — a splash screen loads, then the tray icon appears.
2. Press the hotkey to start recording - default `Ctrl+Alt+P`.
3. Speak - the overlay shows live audio levels.
4. Press the hotkey again to stop - The formatted text is pasted automatically.

---

### Platform Security Notes

Gladys listens to global keystrokes to detect the hotkey and injects keystrokes to paste. Both behaviours are similar to a keylogger, so this table lists exactly what is used under the hood.

| Platform | Hotkey capture (keystrokes in)        | Paste injection (keystrokes out) | Clipboard                | Permissions                   |
| -------- | ------------------------------------- | -------------------------------- | ------------------------ | ----------------------------- |
| Linux    | evdev, reads `/dev/input/event*`      | `/dev/uinput` virtual keyboard   | `wl-copy` (wl-clipboard) | uinput access, `wl-clipboard` |
| Windows  | `SetWindowsHookEx` (`WH_KEYBOARD_LL`) | `SendInput`                      | Qt `QClipboard`          | Run as Administrator          |
| macOS    | `CGEventTap`                          | `CGEvent`                        | Qt `QClipboard`          | Accessibility permission      |

- Audio is captured with [miniaudio](https://miniaud.io/) and transcribed locally. Nothing is sent over the network.
- Transcription history is stored in plaintext in a local SQLite database (`history.db`).
- Keystrokes are only used for the hotkey and the paste shortcut. **Neither Transcription nor Keystroke content is not logged**.

---

# Build

### Prerequisites

| Tool         | Version |
| ------------ | ------- |
| CMake        | 3.21+   |
| C++ compiler | C++17   |
| Qt           | 6.8+    |

#### 1. Install aqtinstall

```bash
pip install aqtinstall
```

#### 2. Install Qt

```bash
scripts/qt-install.sh                                    # host OS/arch (default)
scripts/qt-install.sh --os linux --arch x64              # --os: linux | macos | windows
                                                         # --arch: x64 | arm64
```

Installs into the build space and prints the `QT_ROOT_DIR` export line.

#### 3. Install Linux packages (Linux only)

```bash
# Debian/Ubuntu
sudo apt install libx11-dev wl-clipboard
```

---

#### 4. Set QT_ROOT_DIR

```bash
export QT_ROOT_DIR=$HOME/gladys/qt-6.8.3/gcc_64         # Linux
export QT_ROOT_DIR=$HOME/gladys/qt-6.8.3/macos          # macOS
export QT_ROOT_DIR=$HOME/gladys/qt-6.8.3/msvc2022_64    # Windows
```

#### 5. Clone, fetch dependencies and build

```bash
git clone https://github.com/mjesbar/gladys.git
cd gladys

scripts/build.sh                                        # host OS/arch (default)
scripts/build.sh --os linux --arch x64 --ma-debug off   # --os: linux | macos | windows
                                                        # --arch: x64 | arm64
                                                        # --ma-debug: on | off (miniaudio debug output)
```

Build space (default `$HOME/gladys`, override with `GLADYS_SPACE`):

```txt
$GLADYS_SPACE/
├── build/      CMake binary dir
├── dist/       runtime deps + app
├── release/    packaged archives
└── qt-6.8.3/   Qt install (QT_ROOT_DIR)
```

#### 6. Run

> [!WARNING]
> **Linux: enable paste (uinput)**
>
> ```bash
> scripts/linux/uinput.sh allow   # allow: grant /dev/uinput access (asks for root via pkexec)
> # log out and back in
> scripts/linux/uinput.sh deny    # deny: revoke /dev/uinput access
> ```
>
> Without this step, Gladys couldn't work properly in Wayland.

Run the gladys executable:

```bash
$HOME/gladys/dist/gladys
```

---

# License

[PolyForm Shield License 1.0.0](LICENSE)

---

# Acknowledgements

- [llama.cpp](https://github.com/ggml-org/llama.cpp) — local LLM inference
- [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) — speech-to-text engine
- [miniaudio](https://miniaud.io/) — audio capture library
- [Kompute](https://github.com/KomputeProject/kompute) — Vulkan compute

---

# Contributing

Contributions, issues, and feature requests are welcome.  
Open an [issue](https://github.com/mjesbar/gladys/issues) or submit a pull request.
