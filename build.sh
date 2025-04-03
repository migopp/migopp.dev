#!/bin/bash

mkdir -p target
pandoc -f markdown -t html \
	src/index.md -o target/index.html \
	--template=tmpl/main.tmpl
