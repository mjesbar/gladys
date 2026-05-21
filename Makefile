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
package:
	@# Remove empty c-api directory
	@rm -rf bin/c-api 2>/dev/null || true
	@# Extract LFS archives into place
	@cd bin/llama.cpp && \
		tar xzf llama-b9265-bin-ubuntu-vulkan-x64.tar.gz 2>/dev/null && \
		tar xzf models.tar.gz -C llama-b9265 2>/dev/null || true
	@cd bin/sherpaonnx && \
		tar xjf sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06.tar.bz2 2>/dev/null || true
	@# Remove tarballs
	@find bin/ -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' | xargs rm -f 2>/dev/null || true
	@mkdir -p release
	rsync -a \
		--exclude=cuda \
		--exclude='*.h' \
		--exclude='*.hpp' \
		--exclude='*.sh' \
		--exclude=miniaudio \
		bin/ release/bin/

package-linux: package
	cd release && tar czf gladys-linux-$(ARCH)-$(VERSION).tar.gz bin/

package-macos: package
	cd release && zip -r gladys-macos-$(ARCH)-$(VERSION).zip bin/

package-win: package
	cd release && zip -r gladys-win-$(ARCH)-$(VERSION).zip bin/

clean:
	@rm -rf build release compile_commands.json
