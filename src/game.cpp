#include <engine.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <raylib.h>
#define PLAYER_FORCE 1000

bool isGrounded(const Object &player, const Map &map) {
  Rectangle playerBase = player.hitbox;
  playerBase.y += 1; // Move it down a bit to check for overlap below the player

  // If is on the ground of the map
  if (playerBase.y + playerBase.height >= map.dimensions.height) {
    return true;
  }

  for (const auto &obj : map.objects) {
    if (obj.id == player.id)
      continue; // Skip self

    if (CheckCollisionRecs(playerBase, obj.hitbox)) {
      return true; // Found an object directly below
    }
  }

  return false; // No object found below the player
}

  // Check collision between two rectangles
  // returns 0 if no collision;
  // returns 1 if collision on the right;
  // returns 2 if collision on the left;
  // returns 3 if collision on the top;
  // returns 4 if collision on the bottom;
  //
  int8_t CheckCollisionDirection(Rectangle rect1, Rectangle rect2) {
    if (rect1.x + rect1.width < rect2.x || rect1.x > rect2.x + rect2.width ||
        rect1.y + rect1.height < rect2.y || rect1.y > rect2.y + rect2.height) {
      return 0; // No collision
    }

    float dx = (rect1.x + rect1.width / 2) - (rect2.x + rect2.width / 2);
    float dy = (rect1.y + rect1.height / 2) - (rect2.y + rect2.height / 2);
    float width = (rect1.width + rect2.width) / 2;
    float height = (rect1.height + rect2.height) / 2;
    float crossWidth = width * dy;
    float crossHeight = height * dx;

    if (std::abs(dx) <= width && std::abs(dy) <= height) {
      if (crossWidth > crossHeight) {
        return (crossWidth > -crossHeight) ? 4 : 1;
      } else {
        return (crossWidth > -crossHeight) ? 2 : 3;
      }
    }

    return 0; // No collision
  }

// Resolve collision between two objects considering elastic collision
const Vector2 gravity = {0, 2000.8f}; // Gravity acceleration

int main() {
  // Create a map
  Map map{Dimension{600, 600}, Color{0, 0, 0, 255}};
  // Create a renderer
  Renderer renderer{Dimension{800, 600}, "Game", 60};
  // Create a player
  Object player{
      -1, Rectangle{10, 10, 50, 50}, Vector2{0, 0}, 100,  Vector2{0, 0},
      5,  Color{255, 255, 255, 255}, true,          true, true};
  Object ball{
      1, Rectangle{100, 100, 20, 20}, Vector2{0, 0}, 100,  Vector2{0, 0},
      1, Color{0, 0, 255, 255},       true,          true, true};
  Object heavyBall{
      1,  Rectangle{250, 250, 100, 100}, Vector2{0, 0}, 100,  Vector2{0, 0},
      10, Color{0, 0, 255, 255},         false,         false, false};
  Object elasticBall{
      1,  Rectangle{400, 400, 20, 20}, Vector2{0, 0}, 100,  Vector2{0, 0},
      1, Color{0, 0, 255, 255},        true,          true, true, 0.01f, 1.0f};
  // Add the player to the map
  map.addObject(player);
  map.addObject(ball);
  map.addObject(heavyBall);
  map.addObject(elasticBall);

  auto update = [&](Map &map, float deltaTime) {
      auto& player = map.objects[0];
      auto bigBall = map.objects[2];

      if(CheckCollisionRecs(player.hitbox, bigBall.hitbox)){
          player.color = Color{255, 0, 0, 255};
      } else {
          player.color = Color{255, 255, 255, 255};
      }
  };

  Engine engine;
  engine.renderer = renderer;
  engine.map = map;
  engine.addKeybind(KEY_LEFT, [](Map &map) {
    auto &player = map.objects[0];
    player.acceleration.x += -PLAYER_FORCE;
  });
  engine.addKeybind(KEY_RIGHT, [](Map &map) {
    auto &player = map.objects[0];
    player.acceleration.x += PLAYER_FORCE;
  });

  engine.addKeybind(KEY_UP, [&engine](Map &map) {
    auto &player = map.objects[0];
    player.acceleration.y += -PLAYER_FORCE;
  });
  engine.addKeybind(KEY_DOWN, [](Map &map) {
    auto &player = map.objects[0];
    player.acceleration.y += PLAYER_FORCE;
  });

  engine.addKeybind(KEY_SPACE, [](Map &map) {
    auto &player = map.objects[0];
    if (isGrounded(player, map)) {
      player.speed.y = -500;
    }
  });
  engine.setUpdate(update);

  engine.start();
}
