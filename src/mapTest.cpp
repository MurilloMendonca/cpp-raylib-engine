#include "mapReader.hpp"
#include <raylib.h>
#include <stdio.h>

int main() {
  Map map = readMap("map.txt", 50);
  map.background = {255, 255, 255, 255};
  for (int i = 0; i < map.objects.size(); i++) {
    printf("Object %d:\n", i);
    map.objects[i].print();
  }
  Renderer renderer({1280, 720}, "mapTest", 60);
  Engine engine;
  engine.map = map;
  engine.renderer = renderer;
  int playerIdx = 0;
  for (int i = 0; i < map.objects.size(); i++) {
    if (map.objects[i].id == 1) {
      playerIdx = i;
      break;
    }
  }

  printf("Player index: %d\n", playerIdx);
  Vector2 playerInitPos = {map.objects[playerIdx].hitbox.x,
                           map.objects[playerIdx].hitbox.y};

  engine.addKeybind(KEY_W, [playerIdx](Map &map) {
    map.objects[playerIdx].acceleration.y -= 1000;
  });
  engine.addKeybind(KEY_A, [playerIdx](Map &map) {
    map.objects[playerIdx].acceleration.x -= 1000;
  });
  engine.addKeybind(KEY_S, [playerIdx](Map &map) {
    map.objects[playerIdx].acceleration.y += 1000;
  });
  engine.addKeybind(KEY_D, [playerIdx](Map &map) {
    map.objects[playerIdx].acceleration.x += 1000;
  });

  int points = 0;
  int level = 1;
  engine.setUpdate([playerIdx, &points, &playerInitPos, &level](Map &map, float dt) {
    map.objects[playerIdx].acceleration.y -= 2000;
    if(level==1 && points > 20) {
        level++;
        map = readMap("map2.txt", 50);
        map.background = {255, 255, 255, 255};
        int NewplayerIdx = 0;
        for (int i = 0; i < map.objects.size(); i++) {
          if (map.objects[i].id == 1) {
            NewplayerIdx = i;
            break;
          }
        }
        std::swap(map.objects[playerIdx], map.objects[NewplayerIdx]);
        playerInitPos = {map.objects[playerIdx].hitbox.x,
                           map.objects[playerIdx].hitbox.y};
      points = 0;
    }
    for (auto &obj : map.objects) {
      if (obj.id == 3 && obj.shouldDraw &&
          CheckCollisionRecs(map.objects[playerIdx].hitbox, obj.hitbox)) {
        obj.shouldCollide = false;
        obj.shouldMove = false;
        obj.shouldDraw = false;
        points++;
      }
      if (obj.id == 2 &&
          CheckCollisionRecs(map.objects[playerIdx].hitbox, obj.hitbox)) {
        map.objects[playerIdx].hitbox.x = playerInitPos.x;
        map.objects[playerIdx].hitbox.y = playerInitPos.y;
      }
    }
  });

  engine.setPostDraw([&points, playerIdx](Map &map, float dt) {
    auto playerPos = map.objects[playerIdx].hitbox;
    DrawText(TextFormat("Points: %d", points), playerPos.x, playerPos.y, 20,
             BLACK);
  });
  engine.start(playerIdx);
  return 0;
}
