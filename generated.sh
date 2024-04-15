mkdir -p build
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/footballGame.cpp -o src/footballGame.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/game.cpp -o src/game.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/mapTest.cpp -o src/mapTest.cpp.o
cp resources/* build/
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/footballGame.cpp.o -o build/football -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/game.cpp.o -o build/physics -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger src/mapTest.cpp.o -o build/mapTest -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
