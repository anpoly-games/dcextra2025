#!/bin/sh

mkdir web
rm -rf build && emcmake cmake -S . -D PLATFORM=Web -D GRAPHICS=GRAPHICS_API_OPENGL_ES3 -B build && cmake --build build && cp build/src/game.* web/
