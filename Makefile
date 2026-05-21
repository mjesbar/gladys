.PHONY: build clean run package package-linux package-macos package-win


# Detect host OS
UNAME_S := $(shell uname -s)

# Default target: build for host OS only
build:
	@echo "Building for $(UNAME_S)..."
	@mkdir -p build
	@cmake -B build \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build build --parallel
	@ln -sf build/compile_commands.json . 2>/dev/null || true

run: build
	@cmake --build build --target run

# Package targets for CI release (exclude cuda/, LFS tarballs, headers, scripts)
OS ?= $(UNAME_S)
package:
	@# Remove empty c-api directory
	@rm -rf bin/c-api 2>/dev/null || true
	@# Extract LFS archives into place
	@cd bin/llama.cpp && \
		if [ "$(OS)" = "Linux" ] && [ "$(ARCH)" = "x64" ]; then \
			tar xzf llama-b9265-bin-ubuntu-vulkan-x64.tar.gz; \
		elif [ "$(OS)" = "Linux" ] && [ "$(ARCH)" = "arm64" ]; then \
			tar xzf llama-b9265-bin-ubuntu-vulkan-arm64.tar.gz; \
		elif [ "$(OS)" = "Darwin" ]; then \
			tar xzf llama-b9265-bin-macos-arm64.tar.gz; \
		elif [ "$(OS)" = "Windows_NT" ]; then \
			mkdir -p llama-b9265 && \
			unzip -o llama-b9265-bin-win-vulkan-x64.zip -d llama-b9265; \
		fi 2>/dev/null || true && \
		tar xzf models.tar.gz -C llama-b9265 2>/dev/null || true
	@cd bin/sherpaonnx && \
		tar xjf sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06.tar.bz2 2>/dev/null || true
	@# Remove tarballs
	@find bin/ -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' -o -name '*.zip' | xargs rm -f 2>/dev/null || true
	@mkdir -p release
	rsync -a \
		--exclude=cuda \
		--exclude='*.h' \
		--exclude='*.hpp' \
		--exclude='*.sh' \
		--exclude=miniaudio \
		bin/ release/bin/

package-linux: OS=Linux
package-linux: package
	cd release && tar czf gladys-linux-$(ARCH)-$(VERSION).tar.gz bin/

package-macos: OS=Darwin
package-macos: package
	cd release && zip -r gladys-macos-$(ARCH)-$(VERSION).zip bin/

package-win: OS=Windows_NT
package-win: package
	cd release && zip -r gladys-win-$(ARCH)-$(VERSION).zip bin/

clean:
	@rm -rf build release compile_commands.json
