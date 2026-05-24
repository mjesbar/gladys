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
	@cmake --build build --parallel --config Release
	@ln -sf build/compile_commands.json . 2>/dev/null || true
	@# Copy sherpa-onnx libraries flat next to executable
	@if [ -d "src/include/sherpa-onnx/lib/linux/x64" ]; then \
		cp -n src/include/sherpa-onnx/lib/linux/x64/*.so* dist/ 2>/dev/null || true; \
	elif [ -d "src/include/sherpa-onnx/lib/linux/arm64" ]; then \
		cp -n src/include/sherpa-onnx/lib/linux/arm64/*.so* dist/ 2>/dev/null || true; \
	elif [ -d "src/include/sherpa-onnx/lib/macos/arm64" ]; then \
		cp -n src/include/sherpa-onnx/lib/macos/arm64/*.dylib dist/ 2>/dev/null || true; \
	elif [ -d "src/include/sherpa-onnx/lib/win/x64" ]; then \
		cp -n src/include/sherpa-onnx/lib/win/x64/*.dll dist/ 2>/dev/null || true; \
	fi

run: build
	@cmake --build build --target run

# Package targets for CI release (exclude cuda/, LFS tarballs, headers, scripts)
OS ?= $(UNAME_S)
package:
	@# Remove empty c-api directory
	@rm -rf dist/c-api 2>/dev/null || true
	@# Extract LFS archives into place
	@cd dist/llama.cpp && \
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
	@cd dist/sherpaonnx && \
		tar xjf sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06.tar.bz2 2>/dev/null || true
	@# Remove tarballs
	@find dist/ -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' -o -name '*.zip' | xargs rm -f 2>/dev/null || true
	@# Remove non-native executables
	@if [ "$(OS)" != "Linux" ]; then rm -f dist/gladys; fi
	@if [ "$(OS)" != "Windows_NT" ]; then rm -f dist/gladys.exe; fi
	@if [ "$(OS)" != "Darwin" ]; then rm -rf dist/gladys.app; fi
	@mkdir -p release/gladys-v$(VERSION)
	rsync -a \
		--exclude=cuda \
		--exclude='*.h' \
		--exclude='*.hpp' \
		--exclude='*.sh' \
		--exclude=miniaudio \
		dist/ release/gladys-v$(VERSION)/

package-linux: OS=Linux
package-linux: package
	cd release && tar czf gladys-linux-$(ARCH)-$(VERSION).tar.gz gladys-v$(VERSION)

package-macos: OS=Darwin
package-macos: package
	cd release && zip -r gladys-macos-$(ARCH)-$(VERSION).zip gladys-v$(VERSION)

package-win: OS=Windows_NT
package-win: package
	cd release && zip -r gladys-win-$(ARCH)-$(VERSION).zip gladys-v$(VERSION)

clean:
	@rm -rf build release compile_commands.json
