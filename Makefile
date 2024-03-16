football: build 
	g++ -o build/football src/footballGame.cpp -L./dependencies/raylib/src -lraylib -I./dependencies/raylib/src -I./src -I./dependencies

physics: build
	g++ -o build/physics src/game.cpp -L./src/raylib/src -lraylib -I./src/raylib/src -I./src

map: build
	g++ -o build/mapTest src/mapTest.cpp -L./src/raylib/src -lraylib -I./src/raylib/src -I./src -g
build: 
	mkdir build
	cp src/shader.fs src/map.txt src/map2.txt build/
clean:
	rm -rf build
