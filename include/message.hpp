#pragma once
#include "engine.hpp"
#include <vector>


struct ServerMessage {
    int player1Score;
    int player2Score;
    Vector2 ballPosition;
    Vector2 ballSpeed;
    Vector2 player1Position;
    Vector2 player2Position;
    Vector2 player1Speed;
    Vector2 player2Speed;

    ServerMessage(int player1Score, int player2Score, Vector2 ballPosition, Vector2 ballSpeed, Vector2 player1Position, Vector2 player2Position, Vector2 player1Speed, Vector2 player2Speed)
        : player1Score(player1Score), player2Score(player2Score), ballPosition(ballPosition), ballSpeed(ballSpeed), player1Position(player1Position), player2Position(player2Position), player1Speed(player1Speed), player2Speed(player2Speed) {}
    ServerMessage(Map map, int player1Score, int player2Score) {
        this->player1Score = player1Score;
        this->player2Score = player2Score;
        for (auto &object : map.objects) {
            if (object.id == 0) {
                ballPosition.x = object.hitbox.x;
                ballPosition.y = object.hitbox.y;
                ballSpeed.x = object.speed.x;
                ballSpeed.y = object.speed.y;
            } else if (object.id == 1) {
                player1Position.x = object.hitbox.x;
                player1Position.y = object.hitbox.y;
                player1Speed = object.speed;
            } else if (object.id == 2) {
                player2Position.x = object.hitbox.x;
                player2Position.y = object.hitbox.y;
                player2Speed = object.speed;
            }
        }
    }

    ServerMessage(std::vector<char> serialized){
        player1Score = 0;
        for (int i = 0; i < sizeof(player1Score); ++i) {
            player1Score |= (unsigned int)((unsigned char)serialized[i]) << (i * 8);
        }
        int offset = sizeof(player1Score);
        player2Score = 0;
        for (int i = 0; i < sizeof(player2Score); ++i) {
            player2Score |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        offset += sizeof(player2Score);
        int aux = 0;
        for (int i = 0; i < sizeof(ballPosition.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        ballPosition.x = aux;
        offset += sizeof(ballPosition.x);
        aux = 0;
        for (int i = 0; i < sizeof(ballPosition.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        ballPosition.y = aux;
        aux = 0;
        offset += sizeof(ballPosition.y);
        for (int i = 0; i < sizeof(ballSpeed.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        ballSpeed.x = aux;
        offset += sizeof(ballSpeed.x);
        aux = 0;
        for (int i = 0; i < sizeof(ballSpeed.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        ballSpeed.y = aux;
        offset += sizeof(ballSpeed.y);
        aux = 0;
        for (int i = 0; i < sizeof(player1Position.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player1Position.x = aux;
        offset += sizeof(player1Position.x);
        aux = 0;
        for (int i = 0; i < sizeof(player1Position.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player1Position.y = aux;
        offset += sizeof(player1Position.y);
        aux = 0;
        for (int i = 0; i < sizeof(player2Position.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player2Position.x = aux;
        offset += sizeof(player2Position.x);
        aux = 0;
        for (int i = 0; i < sizeof(player2Position.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player2Position.y = aux;
        offset += sizeof(player2Position.y);
        aux = 0;
        for (int i = 0; i < sizeof(player1Speed.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player1Speed.x = aux;
        offset += sizeof(player1Speed.x);
        aux = 0;
        for (int i = 0; i < sizeof(player1Speed.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player1Speed.y = aux;
        offset += sizeof(player1Speed.y);
        aux = 0;
        for (int i = 0; i < sizeof(player2Speed.x); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player2Speed.x = aux;
        offset += sizeof(player2Speed.x);
        aux = 0;
        for (int i = 0; i < sizeof(player2Speed.y); ++i) {
            aux |= (unsigned int)((unsigned char)serialized[offset + i]) << (i * 8);
        }
        player2Speed.y = aux;
    }

    void updateMap(Map& map) {
        for (auto &object : map.objects) {
            if (object.id == 0) {
                object.hitbox.x = ballPosition.x;
                object.hitbox.y = ballPosition.y;
                object.speed.x = ballSpeed.x;
                object.speed.y = ballSpeed.y;
            } else if (object.id == 1) {
                object.hitbox.x = player1Position.x;
                object.hitbox.y = player1Position.y;
                object.speed = player1Speed;
            } else if (object.id == 2) {
                object.hitbox.x = player2Position.x;
                object.hitbox.y = player2Position.y;
                object.speed = player2Speed;
            }
        }
    }
    std::vector<char> serialize() {
        std::vector<char> serialized;
        for (int i = 0; i < sizeof(player1Score); ++i) {
            serialized.push_back((player1Score >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player2Score); ++i) {
            serialized.push_back((player2Score >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(ballPosition.x); ++i) {
            serialized.push_back(((int)ballPosition.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(ballPosition.y); ++i) {
            serialized.push_back(((int)ballPosition.y >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(ballSpeed.x); ++i) {
            serialized.push_back(((int)ballSpeed.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(ballSpeed.y); ++i) {
            serialized.push_back(((int)ballSpeed.y >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player1Position.x); ++i) {
            serialized.push_back(((int)player1Position.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player1Position.y); ++i) {
            serialized.push_back(((int)player1Position.y >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player2Position.x); ++i) {
            serialized.push_back(((int)player2Position.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player2Position.y); ++i) {
            serialized.push_back(((int)player2Position.y >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player1Speed.x); ++i) {
            serialized.push_back(((int)player1Speed.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player1Speed.y); ++i) {
            serialized.push_back(((int)player1Speed.y >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player2Speed.x); ++i) {
            serialized.push_back(((int)player2Speed.x >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < sizeof(player2Speed.y); ++i) {
            serialized.push_back(((int)player2Speed.y >> (i * 8)) & 0xFF);
        }
        return serialized;
    }
};

struct ClientMessage {
  int acceleration_x;
  int acceleration_y;

  ClientMessage(int acceleration_x, int acceleration_y)
      : acceleration_x(acceleration_x), acceleration_y(acceleration_y) {}
  ClientMessage(std::vector<char> serialized) {
    acceleration_x = 0;
    for (int i = 0; i < sizeof(acceleration_x); ++i) {
      acceleration_x |= (unsigned int)((unsigned char)serialized[i]) << (i * 8);
    }

    acceleration_y = 0;
    for (int i = 0; i < sizeof(acceleration_y); ++i) {
      acceleration_y |=
          (unsigned int)((unsigned char)serialized[i + sizeof(acceleration_x)])
          << (i * 8);
    }
  }
  std::vector<char> serialize() {
    std::vector<char> serialized;
    for (int i = 0; i < sizeof(acceleration_x); ++i) {
      serialized.push_back((acceleration_x >> (i * 8)) & 0xFF);
    }

    for (int i = 0; i < sizeof(acceleration_y); ++i) {
      serialized.push_back((acceleration_y >> (i * 8)) & 0xFF);
    }

    return serialized;
  }

  void print() {
    printf("\nAcceleration in X: %d, Acceleration in Y: %d\n", acceleration_x, acceleration_y);
  }
};
