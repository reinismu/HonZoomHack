#!/bin/sh

mkdir -p _build

cd _build

cmake ..
make VERBOSE=1

mv zoomHack ../
