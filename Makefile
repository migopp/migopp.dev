all: build

build:
	cc -std=gnu2x -g -o build build.c

html: build
	./build

clean:
	rm -rf build
	rm -rf target

.PHONY: all build clean
