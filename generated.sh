mkdir -p build
ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.a build/libraylib.a
ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so build/libraylib.so
ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so.5.0.0 build/libraylib.so.5.0.0
ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so.500 build/libraylib.so.500
cp resources/* build/
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ -c src/footballGame/footballGame.cpp -o src/footballGame/footballGame.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ -c src/footballGame/network.cpp -o src/footballGame/network.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ src/footballGame/footballGame.cpp.o src/footballGame/network.cpp.o -o build/football -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/game.cpp -o src/game.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ src/game.cpp.o -o build/physics -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/mapTest.cpp -o src/mapTest.cpp.o
g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ src/mapTest.cpp.o -o build/mapTest -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
