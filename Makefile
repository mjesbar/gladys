.PHONY: build clean

build:
	@cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build build
	@ln -sf build/compile_commands.json .

clean:
	@rm -rf build compile_commands.json