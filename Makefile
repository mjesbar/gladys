.PHONY: build clean run package package-linux package-macos package-win fetch-models fetch-qt

# -------------------------------------------------------------------
# OS Detection
# -------------------------------------------------------------------
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    OS := Linux
else ifeq ($(UNAME_S),Darwin)
    OS := macOS
else
    OS := Windows
endif

# -------------------------------------------------------------------
# Fetch llama.cpp binaries, llama.cpp model & sherpaonnx model
# -------------------------------------------------------------------
LLAMACPP_DIR    := dist/llama.cpp
SHERPAONNX_DIR  := dist/sherpaonnx
LLAMACPP_VER    := llama-b9265

LLAMACPP_MODEL  := Llama-3.2-1B-Instruct-Q4_K_M.gguf
SHERPAONNX_MODEL := sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06

# URLs from scripts/binaries.yaml
LLAMACPP_URL_LINUX   := https://github.com/ggml-org/llama.cpp/releases/download/b9265/llama-b9265-bin-ubuntu-vulkan-x64.tar.gz
LLAMACPP_URL_MACOS   := https://github.com/ggml-org/llama.cpp/releases/download/b9265/llama-b9265-bin-macos-arm64.tar.gz
LLAMACPP_URL_WINDOWS := https://github.com/ggml-org/llama.cpp/releases/download/b9265/llama-b9265-bin-win-vulkan-x64.zip
LLAMACPP_MODEL_URL   := https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf?download=true
SHERPAONNX_MODEL_URL := https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06.tar.bz2

# Select the right llama.cpp archive URL per OS
ifeq ($(OS),Linux)
    LLAMACPP_ARCHIVE := $(LLAMACPP_URL_LINUX)
    LLAMACPP_FILE    := llama-b9265-bin-ubuntu-vulkan-x64.tar.gz
else ifeq ($(OS),macOS)
    LLAMACPP_ARCHIVE := $(LLAMACPP_URL_MACOS)
    LLAMACPP_FILE    := llama-b9265-bin-macos-arm64.tar.gz
else
    LLAMACPP_ARCHIVE := $(LLAMACPP_URL_WINDOWS)
    LLAMACPP_FILE    := llama-b9265-bin-win-vulkan-x64.zip
endif

fetch-models:
	@echo "=== Fetching models for $(OS) ==="
	@mkdir -p $(LLAMACPP_DIR) $(SHERPAONNX_DIR)

# ---- llama.cpp binary archive ----
	@if [ ! -f "$(LLAMACPP_DIR)/$(LLAMACPP_FILE)" ]; then \
		echo "Downloading llama.cpp ($(OS))..."; \
		curl -L -o "$(LLAMACPP_DIR)/$(LLAMACPP_FILE)" "$(LLAMACPP_ARCHIVE)"; \
	fi

# ---- Extract llama.cpp ----
	@if [ ! -d "$(LLAMACPP_DIR)/$(LLAMACPP_VER)" ]; then \
		echo "Extracting llama.cpp..."; \
		mkdir -p "$(LLAMACPP_DIR)/$(LLAMACPP_VER)"; \
		case "$(OS)" in \
			Windows) \
				unzip -o "$(LLAMACPP_DIR)/$(LLAMACPP_FILE)" -d "$(LLAMACPP_DIR)/$(LLAMACPP_VER)" ;; \
			*) \
				tar xzf "$(LLAMACPP_DIR)/$(LLAMACPP_FILE)" -C "$(LLAMACPP_DIR)/$(LLAMACPP_VER)" --strip-components=1 ;; \
		esac; \
	fi

# ---- llama.cpp model (single .gguf file) ----
	@if [ ! -f "$(LLAMACPP_DIR)/$(LLAMACPP_VER)/models/$(LLAMACPP_MODEL)" ]; then \
		echo "Downloading llama.cpp model..."; \
		mkdir -p "$(LLAMACPP_DIR)/$(LLAMACPP_VER)/models"; \
		curl -L -o "$(LLAMACPP_DIR)/$(LLAMACPP_VER)/models/$(LLAMACPP_MODEL)" "$(LLAMACPP_MODEL_URL)"; \
	fi

# ---- sherpaonnx model archive ----
	@if [ ! -f "$(SHERPAONNX_DIR)/$(SHERPAONNX_MODEL).tar.bz2" ]; then \
		echo "Downloading sherpa-onnx model..."; \
		curl -L -o "$(SHERPAONNX_DIR)/$(SHERPAONNX_MODEL).tar.bz2" "$(SHERPAONNX_MODEL_URL)"; \
	fi

# ---- Extract sherpaonnx ----
	@if [ ! -d "$(SHERPAONNX_DIR)/$(SHERPAONNX_MODEL)" ]; then \
		echo "Extracting sherpa-onnx model..."; \
		tar xjf "$(SHERPAONNX_DIR)/$(SHERPAONNX_MODEL).tar.bz2" -C "$(SHERPAONNX_DIR)"; \
	fi

	@echo "=== All models fetched ==="

# -------------------------------------------------------------------
# Fetch Qt runtime files (libs + plugins + qt.conf)
# -------------------------------------------------------------------
QT_ROOT_DIR ?= /dev/null

dist/qt/lib dist/qt/plugins/platforms dist/qt/plugins/imageformats:
	@mkdir -p dist/qt/lib dist/qt/plugins/platforms dist/qt/plugins/imageformats

fetch-qt: dist/qt/lib dist/qt/plugins/platforms dist/qt/plugins/imageformats
	@echo "=== Fetching Qt runtime for $(OS) ==="
ifeq ($(OS),Linux)
	@find "$(QT_ROOT_DIR)/lib" -name '*.so*' -exec cp -P {} dist/qt/lib/ \;
	@cp -r "$(QT_ROOT_DIR)/plugins/platforms/"*   dist/qt/plugins/platforms/ 2>/dev/null || true
	@cp -r "$(QT_ROOT_DIR)/plugins/imageformats/"* dist/qt/plugins/imageformats/ 2>/dev/null || true
else ifeq ($(OS),macOS)
	@# macOS Qt6 ships as .framework bundles — copy entire frameworks
	@for fw in Core Gui Widgets Network Svg; do \
		cp -R "$(QT_ROOT_DIR)/lib/Qt$${fw}.framework" dist/qt/lib/ 2>/dev/null || true; \
	done
	@cp -r "$(QT_ROOT_DIR)/plugins/platforms/"*   dist/qt/plugins/platforms/ 2>/dev/null || true
	@cp -r "$(QT_ROOT_DIR)/plugins/imageformats/"* dist/qt/plugins/imageformats/ 2>/dev/null || true
else
	@# Windows: .dll files live in bin/ (and some in lib/)
	@cp "$(QT_ROOT_DIR)/bin/"*.dll   dist/qt/lib/ 2>/dev/null || true
	@cp "$(QT_ROOT_DIR)/lib/"*.dll   dist/qt/lib/ 2>/dev/null || true
	@cp -r "$(QT_ROOT_DIR)/plugins/platforms/"*   dist/qt/plugins/platforms/ 2>/dev/null || true
	@cp -r "$(QT_ROOT_DIR)/plugins/imageformats/"* dist/qt/plugins/imageformats/ 2>/dev/null || true
endif
	@# Write qt.conf — paths relative to Prefix (.)
	@printf '[Paths]\nPrefix = .\nPlugins = qt/plugins\nLibraries = qt/lib\n' > dist/qt.conf
	@echo "=== Qt runtime fetched ==="

# -------------------------------------------------------------------
# Build
# -------------------------------------------------------------------
build:
	@echo "Building for $(OS)..."
	@mkdir -p build
	@cmake -B build \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build build --parallel --config Release
	@ln -sf build/compile_commands.json . 2>/dev/null || true

run: build
	@cmake --build build --target run

# -------------------------------------------------------------------
# Package — fetch-models runs first
# -------------------------------------------------------------------
OS_OVERRIDE ?= $(OS)

package: fetch-models fetch-qt
	@# Remove empty c-api directory
	@rm -rf dist/c-api 2>/dev/null || true
	@# Remove tarballs
	@find dist/ -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' -o -name '*.zip' | xargs rm -f 2>/dev/null || true
	@# Remove non-native executables
	@if [ "$(OS_OVERRIDE)" != "Linux" ] && [ "$(OS_OVERRIDE)" != "macOS" ]; then rm -f dist/gladys; fi
	@if [ "$(OS_OVERRIDE)" != "Windows" ]; then rm -f dist/gladys.exe; fi
	@mkdir -p release/gladys-v$(VERSION)
	rsync -a \
		--exclude=cuda \
		--exclude='*.h' \
		--exclude='*.hpp' \
		--exclude='*.sh' \
		--exclude=miniaudio \
		dist/ release/gladys-v$(VERSION)/

package-linux: OS_OVERRIDE=Linux
package-linux: package
	cd release && tar czf gladys-linux-$(ARCH)-$(VERSION).tar.gz gladys-v$(VERSION)

package-macos: OS_OVERRIDE=macOS
package-macos: package
	cd release && zip -r gladys-macos-$(ARCH)-$(VERSION).zip gladys-v$(VERSION)

package-win: OS_OVERRIDE=Windows
package-win: package
	cd release && zip -r gladys-win-$(ARCH)-$(VERSION).zip gladys-v$(VERSION)

# -------------------------------------------------------------------
# Clean
# -------------------------------------------------------------------
clean:
	@rm -rf build release compile_commands.json
