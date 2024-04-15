CC = g++
CFLAGS = 

all: buildDir dynamicLibs staticLibs executables
buildDir:
	mkdir -p build
dynamicLibs: 
staticLibs: 
executables: build/football build/physics build/mapTest 
src/footballGame.cpp.o:
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/footballGame.cpp -o src/footballGame.cpp.o
src/game.cpp.o:
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/game.cpp -o src/game.cpp.o
src/mapTest.cpp.o:
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/mapTest.cpp -o src/mapTest.cpp.o
build/football: src/footballGame.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/footballGame.cpp.o -o build/football -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
build/physics: src/game.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/game.cpp.o -o build/physics -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
build/mapTest: src/mapTest.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/mapTest.cpp.o -o build/mapTest -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build