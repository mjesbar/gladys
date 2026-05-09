.PHONY: build clean link run

build:
	@cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@cmake --build build
	@ln -sf build/compile_commands.json .

link: build
	@cmake --build build --target link

run: build
	@cmake --build build --target run

clean:
	@rm -rf build compile_commands.json
