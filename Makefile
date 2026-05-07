.PHONY: build clean

build:
	@cmake -B build -DCMAKE_BUILD_TYPE=Release
	@cmake --build build

clean:
	@rm -rf build