#pragma once
#include <cmath>
extern "C" {
#include "raylib.h"
}

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "rlgl.h"

struct Point {
  int x;
  int y;

  Point(int x, int y) : x(x), y(y) {}

  Vector2 getVec2() { return Vector2{(float)x, (float)y}; }
};

struct Dimension {
  int width;
  int height;

  Dimension() : width(0), height(0) {}
  Dimension(int width, int height) : width(width), height(height) {}

  Dimension(Vector2 vec2) : width((int)vec2.x), height((int)vec2.y) {}

  Vector2 getVec2() { return Vector2{(float)width, (float)height}; }
};

struct Object {
  int id;
  Rectangle hitbox;
  Vector2 speed;
  float maxSpeed;
  Vector2 direction;
  float weight;
  Color color;
  bool shouldMove;
  bool shouldCollide;
  bool shouldDraw;
  Vector2 acceleration = Vector2{0, 0};
  float friction = 0.01;
  float elasticity = 0.5;
  bool isCircle = false;

  void print() {
    printf("Object: %d\n", id);
    printf("Hitbox: %f, %f, %f, %f\n", hitbox.x, hitbox.y, hitbox.width,
           hitbox.height);
    printf("Speed: %f, %f\n", speed.x, speed.y);
    printf("MaxSpeed: %f\n", maxSpeed);
    printf("Direction: %f, %f\n", direction.x, direction.y);
    printf("Weight: %f\n", weight);
    printf("Color: %d, %d, %d, %d\n", color.r, color.g, color.b, color.a);
    printf("ShouldMove: %d\n", shouldMove);
    printf("ShouldCollide: %d\n", shouldCollide);
    printf("ShouldDraw: %d\n", shouldDraw);
    printf("Acceleration: %f, %f\n", acceleration.x, acceleration.y);
    printf("Friction: %f\n", friction);
    printf("Elasticity: %f\n", elasticity);
  }
};

struct Map {
  Dimension dimensions;
  std::vector<Object> objects;
  Color background;

  Map() {}
  Map(Dimension dim, Color col) : dimensions(dim), background(col) {}

  void addObject(Object obj) { objects.push_back(obj); }
  void print() {
    for (auto obj : objects) {
      obj.print();
    }
  }
};

struct Renderer {
  Dimension dimensions;
  std::string title;
  int fps;
  Shader shader;
  Renderer() {}
  Renderer(Dimension dim, std::string title, int fps)
      : dimensions(dim), title(title), fps(fps) {}
  ~Renderer() { close(); }

  void init() {
    InitWindow(dimensions.width, dimensions.height, title.c_str());
    SetTargetFPS(fps);
    shader = LoadShader(0, "shader.fs");
  }

  void close() { CloseWindow(); }

  bool shouldClose() { return WindowShouldClose(); }

  void beginDrawing(Camera2D camera) {

    BeginDrawing();
    BeginMode2D(camera);
  }

  void endDrawing() {
    EndMode2D();
    EndDrawing();
  }

  void drawMap(Map map) {
    ClearBackground(map.background);
    // Draw line around the map
    Texture2D texture = {rlGetTextureIdDefault(), 1, 1, 1,
                         PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    DrawRectangleLines(0, 0, map.dimensions.width, map.dimensions.height, RED);
    for (Object obj : map.objects) {
      if (obj.shouldDraw) {
        if (obj.isCircle) {
          BeginShaderMode(shader);
          float radius = obj.hitbox.width / 2;
          DrawTextureEx(texture,
                        {obj.hitbox.x - radius / 2, obj.hitbox.y - radius / 2},
                        0, 1.5 * obj.hitbox.width, obj.color);
          EndShaderMode();
        } else {
          DrawRectangleRec(obj.hitbox, obj.color);
        }
      }
    }
  }
};

struct Engine {
  Renderer renderer;
  Map map;
  std::unordered_map<uint, std::function<void(Map &)>> keybinds;
  std::function<void(Map &, float)> update;
  std::function<void(Map &, float)> preDraw;
  std::function<void(Map &, float)> postDraw;

  Engine() {}
  void setPreDraw(std::function<void(Map &, float)> func) { preDraw = func; }
  void setPostDraw(std::function<void(Map &, float)> func) { postDraw = func; }
  Engine(Renderer rend, Map m) : renderer(rend), map(m) {}
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
  void correctBoundaryCollision(Rectangle &futureHitbox, Object &object,
                                const Map &map, float deltaTime) {
    // Check collision with the left or right map boundary
    if (futureHitbox.x < 0 ||
        futureHitbox.x + futureHitbox.width > map.dimensions.width) {
      // Reverse horizontal speed and apply elasticity
      object.speed.x = -object.speed.x * object.elasticity;

      // Correct the position to prevent sticking to the boundary
      if (futureHitbox.x < 0) {
        futureHitbox.x = 0;
      } else {
        futureHitbox.x = map.dimensions.width - futureHitbox.width;
      }
    }

    // Check collision with the top or bottom map boundary
    if (futureHitbox.y < 0 ||
        futureHitbox.y + futureHitbox.height > map.dimensions.height) {
      // Reverse vertical speed and apply elasticity
      object.speed.y = -object.speed.y * object.elasticity;

      // Correct the position to prevent sticking to the boundary
      if (futureHitbox.y < 0) {
        futureHitbox.y = 0;
      } else {
        futureHitbox.y = map.dimensions.height - futureHitbox.height;
      }
    }

    // Optionally, handle complete stop for low elasticity impacts
    if (object.elasticity < 0.1f) {
      if (fabs(object.speed.x) < 0.01f)
        object.speed.x = 0;
      if (fabs(object.speed.y) < 0.01f)
        object.speed.y = 0;
    }
  }

  void applySpeedDecay(Object &object) {
    // Assuming friction acts in both x and y directions and affects the speed
    // linearly
    float decayFactorX = 1.0f - object.friction; // Decay factor for x direction
    float decayFactorY = 1.0f - object.friction; // Decay factor for y direction

    // Apply the decay to the object's speed
    object.speed.x *= decayFactorX;
    object.speed.y *= decayFactorY;

    // Optionally, ensure that very small speeds are set to zero to avoid
    // objects moving indefinitely due to tiny speed values
    if (fabs(object.speed.x) < 10.0f)
      object.speed.x = 0;
    if (fabs(object.speed.y) < 10.0f)
      object.speed.y = 0;
  }
  void resolveObjectCollisions(std::vector<Object> &objects, float deltaTime) {
    // Enhanced collision resolution logic incorporating elasticity and friction
    for (size_t i = 0; i < map.objects.size(); ++i) {
      for (size_t j = i + 1; j < map.objects.size(); ++j) {
        Object &obj1 = map.objects[i];
        Object &obj2 = map.objects[j];

        if (!obj1.shouldCollide || !obj2.shouldCollide)
          continue;

        // Predict future positions
        Rectangle futureHitbox1 = obj1.hitbox;
        futureHitbox1.x += obj1.speed.x * deltaTime;
        futureHitbox1.y += obj1.speed.y * deltaTime;

        Rectangle futureHitbox2 = obj2.hitbox;
        futureHitbox2.x += obj2.speed.x * deltaTime;
        futureHitbox2.y += obj2.speed.y * deltaTime;

        // Detect collision direction
        int8_t collisionDirection =
            CheckCollisionDirection(futureHitbox1, futureHitbox2);
        if (collisionDirection) {
          float massTotal = obj1.weight + obj2.weight;
          float transferRatio1 = obj2.weight / massTotal;
          float transferRatio2 = obj1.weight / massTotal;

          // Calculate overlap to adjust positions
          float overlapX = 0.0f, overlapY = 0.0f;
          if (collisionDirection == 1 ||
              collisionDirection == 2) { // Horizontal collision
            overlapX =
                (collisionDirection == 1)
                    ? futureHitbox1.x + futureHitbox1.width - futureHitbox2.x
                    : futureHitbox2.x + futureHitbox2.width - futureHitbox1.x;
          } else { // Vertical collision
            overlapY =
                (collisionDirection == 3)
                    ? futureHitbox1.y + futureHitbox1.height - futureHitbox2.y
                    : futureHitbox2.y + futureHitbox2.height - futureHitbox1.y;
          }

          // Momentum transfer and speed adjustment
          if (collisionDirection == 3 ||
              collisionDirection == 4) { // Vertical collision
            float speedYTotal =
                obj1.speed.y * obj1.weight + obj2.speed.y * obj2.weight;
            obj1.speed.y = speedYTotal * transferRatio1 / obj1.weight;
            obj2.speed.y = speedYTotal * transferRatio2 / obj2.weight;
            // Adjust positions to avoid overlap
            if (collisionDirection == 3) { // Top collision
              if (obj2.shouldMove && obj1.shouldMove) {
                obj2.hitbox.y += overlapY * transferRatio1;
                obj1.hitbox.y -= overlapY * transferRatio2;
              } else {
                if (obj2.shouldMove) {
                  obj2.hitbox.y = obj1.hitbox.y + obj1.hitbox.height;
                } else {
                  obj1.hitbox.y = obj2.hitbox.y - obj1.hitbox.height;
                }
              }
            } else { // Bottom collision
              if (obj2.shouldMove && obj1.shouldMove) {
                obj2.hitbox.y -= overlapY * transferRatio1;
                obj1.hitbox.y += overlapY * transferRatio2;
              } else {
                if (obj2.shouldMove) {
                  obj2.hitbox.y = obj1.hitbox.y - obj2.hitbox.height;
                } else {
                  obj1.hitbox.y = obj2.hitbox.y + obj2.hitbox.height;
                }
              }
            }
          } else { // Horizontal collision
            float speedXTotal =
                obj1.speed.x * obj1.weight + obj2.speed.x * obj2.weight;
            obj1.speed.x = speedXTotal * transferRatio1 / obj1.weight;
            obj2.speed.x = speedXTotal * transferRatio2 / obj2.weight;
            // Adjust positions to avoid overlap
            if (collisionDirection == 1) { // Left collision
              if (obj2.shouldMove && obj1.shouldMove) {
                obj2.hitbox.x += overlapX * transferRatio1;
                obj1.hitbox.x -= overlapX * transferRatio2;
              } else {
                if (obj2.shouldMove) {
                  obj2.hitbox.x += overlapX;
                } else {
                  obj1.hitbox.x -= overlapX;
                }
              }
            } else { // Right collision
              if (obj2.shouldMove && obj1.shouldMove) {
                obj2.hitbox.x -= overlapX * transferRatio1;
                obj1.hitbox.x += overlapX * transferRatio2;
              } else {
                if (obj2.shouldMove) {
                  obj2.hitbox.x -= overlapX;
                } else {
                  obj1.hitbox.x += overlapX;
                }
              }
            }
          }
        }
      }
    }
  }
  void simulatePhysics(float deltaTime) {
    for (auto &object : map.objects) {
      if (!object.shouldMove)
        continue;

      // Update speed based on acceleration and apply friction
      object.speed.x += object.acceleration.x * deltaTime -
                        object.speed.x * object.friction * deltaTime;
      object.speed.y += object.acceleration.y * deltaTime -
                        object.speed.y * object.friction * deltaTime;

      // Predictive future position
      Rectangle futureHitbox = object.hitbox;
      futureHitbox.x += object.speed.x * deltaTime;
      futureHitbox.y += object.speed.y * deltaTime;

      // Collision with map boundaries prediction and correction
      correctBoundaryCollision(futureHitbox, object, map, deltaTime);

      // Apply predicted corrections
      object.hitbox = futureHitbox;
      object.acceleration = {0, 0}; // Reset acceleration

      // Speed decay after position update
      applySpeedDecay(object);
    }

    // Predictive object-to-object collision detection and resolution
    resolveObjectCollisions(map.objects, deltaTime);
  }

  void addKeybind(uint key, std::function<void(Map &)> func) {
    keybinds[key] = func;
  }
  void setUpdate(std::function<void(Map &, float)> func) { update = func; }

  void start(int ObjIdToCenterOn = 0) {
    renderer.init();
    while (!renderer.shouldClose()) {
      for (auto const &[key, val] : keybinds) {
        if (IsKeyDown(key)) {
          val(map);
        }
      }
      auto player = map.objects[ObjIdToCenterOn];
      auto speedStr = "Player.speed: " + std::to_string(player.speed.x) + "," +
                      std::to_string(player.speed.y);
      auto accStr =
          "Player.acceleration: " + std::to_string(player.acceleration.x) +

          ", " + std::to_string(player.acceleration.y);
      if (update) {
        update(map, GetFrameTime());
      }

      simulatePhysics(GetFrameTime());
      auto playerCenter = Vector2{player.hitbox.x + player.hitbox.width / 2,
                                  player.hitbox.y + player.hitbox.height / 2};
      Camera2D camera = {0};
      camera.target = playerCenter;
      camera.offset = Vector2{renderer.dimensions.width / 2.0f,
                              renderer.dimensions.height / 2.0f};
      camera.rotation = 0.0f;
      camera.zoom = 1.0f;
      renderer.beginDrawing(camera);

      if (preDraw) {
        preDraw(map, GetFrameTime());
      }
      renderer.drawMap(map);

      if (postDraw) {
        postDraw(map, GetFrameTime());
      }

      DrawText(speedStr.c_str(), 10, 10, 20, BLACK);
      DrawText(accStr.c_str(), 10, 30, 20, BLACK);

      renderer.endDrawing();
    }
  }
};
