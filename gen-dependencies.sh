#!/bin/bash
mkdir -p dependencies/raylib
wget https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_linux_amd64.tar.gz -O dependencies/raylib.tar.gz
tar -xvzf dependencies/raylib.tar.gz -C dependencies/raylib --strip-components=1
rm dependencies/raylib.tar.gz
mkdir -p dependencies/raygui
wget https://github.com/raysan5/raygui/archive/refs/tags/4.0.tar.gz -O dependencies/raygui.tar.gz
tar -xvzf dependencies/raygui.tar.gz -C dependencies/raygui --strip-components=1
rm dependencies/raygui.tar.gz
mkdir -p dependencies/logger
wget https://raw.githubusercontent.com/MurilloMendonca/tiny-libs/master/logger.h -O dependencies/logger/logger.h
