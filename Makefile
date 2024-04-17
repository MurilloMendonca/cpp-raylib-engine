all: buildDir handleDependencies dynamicLibs staticLibs executables
buildDir:
	mkdir -p build
dynamicLibs: 
staticLibs: 
executables: copyResourcesToBuildDir build/football build/physics build/mapTest 
handleDependencies: 
	ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.a build/libraylib.a
	ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so build/libraylib.so
	ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so.5.0.0 build/libraylib.so.5.0.0
	ln -sf /home/gengar/codes/cpp-raylib-engine/dependencies/raylib/liblibraylib.so.500 build/libraylib.so.500
copyResourcesToBuildDir: dynamicLibs
	cp resources/* build/
build/football: src/footballGame/footballGame.cpp src/footballGame/network.cpp dynamicLibs
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ -c src/footballGame/footballGame.cpp -o src/footballGame/footballGame.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ -c src/footballGame/network.cpp -o src/footballGame/network.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -Iinclude/footballGame/ src/footballGame/footballGame.cpp.o src/footballGame/network.cpp.o -o build/football -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
build/physics: src/game.cpp dynamicLibs
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/game.cpp -o src/game.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ src/game.cpp.o -o build/physics -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
build/mapTest: src/mapTest.cpp dynamicLibs
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ -c src/mapTest.cpp -o src/mapTest.cpp.o
	g++  -Idependencies/raylib/include -Idependencies/raygui/src -Idependencies/logger -Iinclude/ src/mapTest.cpp.o -o build/mapTest -Lbuild -lraylib -Wl,-rpath=/home/gengar/codes/cpp-raylib-engine/build
clean:
	rm -rf build/
	rm -f src/footballGame/footballGame.cpp.o
	rm -f src/footballGame/network.cpp.o
	rm -f src/game.cpp.o
	rm -f src/mapTest.cpp.o