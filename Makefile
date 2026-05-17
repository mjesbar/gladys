.PHONY: build clean link run linux macos windows all

# Detect host OS
UNAME_S := $(shell uname -s)

# Default target: build for host OS only
build:
	@echo "Building for $(UNAME_S)..."
	@mkdir -p build/linux
	@cmake -B build/linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build build/linux
	@ln -sf build/linux/compile_commands.json .

# Cross-compile for Linux (native or cross)
linux:
	@echo "Building for Linux..."
	@mkdir -p build/linux
	@cmake -B build/linux -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_SYSTEM_NAME=Linux \
		$(LINUX_TOOLCHAIN)
	@cmake --build build/linux

# Cross-compile for macOS
macos:
	@echo "Building for macOS..."
	@mkdir -p build/macos
	@cmake -B build/macos -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_SYSTEM_NAME=Darwin \
		$(MACOS_TOOLCHAIN)
	@cmake --build build/macos

# Cross-compile for Windows
windows:
	@echo "Building for Windows..."
	@mkdir -p build/win
	@cmake -B build/win -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_SYSTEM_NAME=Windows \
		$(WINDOWS_TOOLCHAIN)
	@cmake --build build/win

# Build all three targets
all: linux macos windows

link: build
	@cmake --build build/linux --target link

run: build
	@cmake --build build/linux --target run

clean:
	@rm -rf build compile_commands.json
