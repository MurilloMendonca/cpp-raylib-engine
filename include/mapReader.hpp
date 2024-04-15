#pragma once
#include "engine.hpp"
#include <fstream>
#include <map>
#include <string>

const Object EMPTY = Object{.id = 0,
                            .hitbox = {0, 0, 0, 0},
                            .speed = {0, 0},
                            .maxSpeed = 0,
                            .direction = {0, 0},
                            .weight = 0,
                            .color = {0, 0, 0, 0},
                            .shouldMove = false,
                            .shouldCollide = false,
                            .shouldDraw = false,
                            .acceleration = {0, 0},
                            .friction = 0,
                            .elasticity = 0,
                            .isCircle = false};
const Object PLAYER = Object{.id = 1,
                             .hitbox = {0, 0, 32, 32},
                             .speed = {0, 0},
                             .weight = 1,
                             .color = RED,
                             .shouldMove = true,
                             .shouldCollide = true,
                             .shouldDraw = true,
                             .friction = 0.05f,
                             .elasticity = 1.0f,
                             .isCircle = false};

const Object WALL = Object{.id = 2,
                           .hitbox = {0, 0, 32, 32},
                           .speed = {0, 0},
                           .maxSpeed = 0,
                           .direction = {0, 0},
                           .weight = 1,
                           .color = {0, 0, 255, 255},
                           .shouldMove = false,
                           .shouldCollide = false,
                           .shouldDraw = true,
                           .acceleration = {0, 0},
                           .friction = 0,
                           .elasticity = 0,
                           .isCircle = false};
const Object BOX = Object{.id = 3,
                           .hitbox = {0, 0, 32, 32},
                           .speed = {0, 0},
                           .maxSpeed = 0,
                           .direction = {0, 0},
                           .weight = 2,
                           .color = {0, 0, 100, 255},
                           .shouldMove = true,
                           .shouldCollide = false,
                           .shouldDraw = true,
                           .acceleration = {0, 0},
                           .friction = 0,
                           .elasticity = 0.5f,
                           .isCircle = false};
const std::unordered_map<char, Object> standardKeys{
                            {'#', WALL}, 
                            {'@', PLAYER},
                            {' ', EMPTY},
                            {'*', BOX}
};
Map readMap(const std::string &filename, float gridScale = 32,
            const std::unordered_map<char, Object> &keys = standardKeys) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    throw std::runtime_error("File not found");
  }

  Map map;
  std::string line;
  int xPadding = 10;
  int yPadding = 10;
  int y = 0;
  int width = 0;
  while (std::getline(file, line)) {
    if (line.size() > width) {
      width = line.size();
    }
    for (int x = 0; x < line.size(); x++) {
      Object o = keys.at(line[x]);
      o.hitbox = {xPadding + x * gridScale, yPadding + y * gridScale, gridScale,
                  gridScale};
      map.addObject(o);
    }
    y++;
  }

  map.dimensions.width = 2 * xPadding + width * gridScale;
  map.dimensions.height = 2 * yPadding + y * gridScale;
  return map;
}
