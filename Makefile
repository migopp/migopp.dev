all: build

build:
	cc -std=gnu2x -o build build.c

html: build
	./build

clean:
	rm -rf build
	rm -rf target

.PHONY: all build clean
