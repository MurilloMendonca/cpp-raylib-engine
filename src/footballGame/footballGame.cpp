#include "common.hpp"
#include "engine.hpp"
#include "footballGame/network.hpp"
#include "raylib.h"
#include "rlgl.h"
#include <thread>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

/* ============================
 * Constants
 * ============================*/
const int PLAYER_SIZE = 50;
const Dimension FIELD_DIMENSION = {600, 800};
const Dimension WINDOW_DIMENSION = {800, 1000};
const Vector2 PLAYER_1_START_POS = {
    (FIELD_DIMENSION.width - PLAYER_SIZE) / 2.0f, 100};
const Vector2 PLAYER_2_START_POS = {PLAYER_1_START_POS.x, 650};
const Dimension GOAL_DIMENSION = {200, 10};
const Vector2 GOAL_1_POS = {200, 0};
const Vector2 GOAL_2_POS = {200, 800.0f - GOAL_DIMENSION.height};
const int BALL_SIZE = 20;
const Vector2 BALL_START_POS = {(FIELD_DIMENSION.width - BALL_SIZE) / 2.0f,
                                (FIELD_DIMENSION.height - BALL_SIZE) / 2.0f};
const Rectangle IP_FIELD_RECT = {0, FIELD_DIMENSION.height + 50.0f, 200, 50};
const Rectangle CONNECT_BUTTON_RECT = {200, FIELD_DIMENSION.height + 50.0f, 100,
                                       50};
const Rectangle START_SERVER_BUTTON_RECT = {400, FIELD_DIMENSION.height + 50.0f,
                                            100, 50};
const Color FIELD_COLOR = {65, 168, 63, 255};

enum class Mode { SERVER, CLIENT, STANDALONE };
enum class Direction { UP, DOWN, LEFT, RIGHT };
enum class ObjectType { BALL = 0, PLAYER1, PLAYER2, GOAL, CENTER };

/* ============================
 * Global variables / state
 * ============================*/
int player1Score = 0;
int player2Score = 0;
bool editText = false;
char userText[100] = {0};

// ==============================
// Function declarations
// ==============================
void setupStandaloneModeKeybinds(Engine &engine);
void enterClientMode(int sock, Engine &engine);
void enterServerMode(int sock, Engine &engine);
void setupServerModeKeybinds(Engine &engine);
void setupClientModeKeybinds(Engine &engine);
void setupKeybinds(Engine &engine, Mode mode, int sock);
void resetGame(Map &map);

// ==============================
// Connection related functions
// ==============================

void updateMap(ServerMessageQueuePtr statusQueue, Map &map) {
  if (!statusQueue->empty()) {
    ServerMessage status = statusQueue->front();
    statusQueue->pop();
    status.updateMap(map);
    player1Score = status.player1Score;
    player2Score = status.player2Score;
  }
}

// ==============================
// Game logic related functions
// ==============================
void resetGame(Map &map) {
  auto &ball = map.objects[3];
  auto &player1 = map.objects[1];
  auto &player2 = map.objects[2];
  ball.hitbox.x = BALL_START_POS.x;
  ball.hitbox.y = BALL_START_POS.y;
  ball.speed.x = 0;
  ball.speed.y = 0;
  player1.hitbox.x = PLAYER_1_START_POS.x;
  player1.hitbox.y = PLAYER_1_START_POS.y;
  player2.speed = {0, 0};
  player2.hitbox.x = PLAYER_2_START_POS.x;
  player2.hitbox.y = PLAYER_2_START_POS.y;
  player2.speed = {0, 0};
}
void setupStandaloneModeKeybinds(Engine &engine) {
  engine.addKeybind(KEY_ENTER, [&engine](Map &map) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    editText = !editText;
    auto sock = clientStablishConnection(userText);
    if (!sock.has_value()) {
      printf("Error stablishing the connection\n");
      return;
    }
    enterClientMode(sock.value(), engine);
  });

  engine.addKeybind(KEY_P, [&engine](Map &map) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto socket = serverStablishConnection();
    if (!socket.has_value()) {
      printf("Error stablishing the connection\n");
      return;
    }
    enterServerMode(socket.value(), engine);
  });
  engine.addKeybind(KEY_TAB, [](Map &map) {
    editText = !editText;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });
  engine.addKeybind(KEY_SPACE, [](Map &map) {
    resetGame(map);
    player1Score = 0;
    player2Score = 0;
  });
  engine.addKeybind(KEY_A, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.x -= 1000;
  });

  engine.addKeybind(KEY_W, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.y -= 1000;
  });

  engine.addKeybind(KEY_D, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.x += 1000;
  });

  engine.addKeybind(KEY_S, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.y += 1000;
  });

  engine.addKeybind(KEY_LEFT, [](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.x -= 1000;
  });

  engine.addKeybind(KEY_UP, [](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.y -= 1000;
  });

  engine.addKeybind(KEY_RIGHT, [](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.x += 1000;
  });

  engine.addKeybind(KEY_DOWN, [](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.y += 1000;
  });
}

void setupServerModeKeybinds(Engine &engine) {
  engine.addKeybind(KEY_A, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.x -= 1000;
  });

  engine.addKeybind(KEY_W, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.y -= 1000;
  });

  engine.addKeybind(KEY_D, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.x += 1000;
  });

  engine.addKeybind(KEY_S, [](Map &map) {
    auto &player = map.objects[1];
    player.acceleration.y += 1000;
  });

  engine.addKeybind(KEY_SPACE, [](Map &map) { resetGame(map); });

  // clear the keybinds of the player 2
  engine.addKeybind(KEY_LEFT, [](Map &map) {});
  engine.addKeybind(KEY_UP, [](Map &map) {});
  engine.addKeybind(KEY_RIGHT, [](Map &map) {});
  engine.addKeybind(KEY_DOWN, [](Map &map) {});
  // clear the keybinds of connection
  engine.addKeybind(KEY_ENTER, [](Map &map) {});
  engine.addKeybind(KEY_P, [](Map &map) {});
  engine.addKeybind(KEY_TAB, [](Map &map) {});
}

void setupClientModeKeybinds(Engine &engine, int sock) {
  engine.addKeybind(KEY_A, [sock](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.x -= 1000;
  });

  engine.addKeybind(KEY_W, [sock](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.y -= 1000;
  });

  engine.addKeybind(KEY_D, [sock](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.x += 1000;
  });

  engine.addKeybind(KEY_S, [sock](Map &map) {
    auto &player = map.objects[2];
    player.acceleration.y += 1000;
  });

  // clear the keybinds of the player 1
  engine.addKeybind(KEY_LEFT, [](Map &map) {});
  engine.addKeybind(KEY_UP, [](Map &map) {});
  engine.addKeybind(KEY_RIGHT, [](Map &map) {});
  engine.addKeybind(KEY_DOWN, [](Map &map) {});
  // clear the keybinds of connection
  engine.addKeybind(KEY_ENTER, [](Map &map) {});
  engine.addKeybind(KEY_P, [](Map &map) {});
  engine.addKeybind(KEY_TAB, [](Map &map) {});
  engine.addKeybind(KEY_SPACE, [](Map &map) {});
}

void setupKeybinds(Engine &engine, Mode mode, int sock = 0) {
  if (mode == Mode::STANDALONE) {
    setupStandaloneModeKeybinds(engine);
  } else if (mode == Mode::SERVER) {
    setupServerModeKeybinds(engine);
  } else if (mode == Mode::CLIENT) {
    setupClientModeKeybinds(engine, sock);
  }
}

void enterClientMode(int sock, Engine &engine) {
  player1Score = 0;
  player2Score = 0;

  auto statusQueue = std::make_shared<std::queue<ServerMessage>>();
  auto messageQueue = std::make_shared<std::queue<ClientMessage>>();

  std::thread clientThread(receiveServerMessage, sock, statusQueue);
  clientThread.detach();

  std::thread sendThread(sendClientMessages, sock, messageQueue);
  sendThread.detach();

  auto update = [sock, statusQueue, messageQueue](Map &map, float deltaTime) {
    if (map.objects[2].acceleration.x != 0 ||
        map.objects[2].acceleration.y != 0) {
      ClientMessage cMessage(map.objects[2].acceleration.x,
                             map.objects[2].acceleration.y);
      enqueueClientMessage(messageQueue, cMessage);
    }

    updateMap(statusQueue, map);

    auto &ball = map.objects[3];
    auto &player1 = map.objects[2];
    auto &player2 = map.objects[1];
    auto &goal1 = map.objects[4];
    auto &goal2 = map.objects[5];
    if (CheckCollisionRecs(ball.hitbox, goal1.hitbox)) {
      resetGame(map);
      player2Score++;
    }
    if (CheckCollisionRecs(ball.hitbox, goal2.hitbox)) {
      resetGame(map);
      player1Score++;
    }
  };

  setupKeybinds(engine, Mode::CLIENT, sock);
  engine.setUpdate(update);
}

void enterServerMode(int sock, Engine &engine) {
  auto messageQueue = std::make_shared<std::queue<ClientMessage>>();
  auto statusQueue = std::make_shared<std::queue<ServerMessage>>();
  player1Score = 0;
  player2Score = 0;

  std::thread serverThread(receiveClientMessage, sock, messageQueue);
  serverThread.detach();

  std::thread sendThread(sendServerMessage, sock, statusQueue);
  sendThread.detach();

  auto update = [sock, messageQueue, statusQueue](Map &map, float deltaTime) {
    auto &ball = map.objects[3];
    auto &player1 = map.objects[1];
    auto &player2 = map.objects[2];
    auto &goal1 = map.objects[4];
    auto &goal2 = map.objects[5];
    if (CheckCollisionRecs(ball.hitbox, goal1.hitbox)) {
      resetGame(map);
      player2Score++;
    }
    if (CheckCollisionRecs(ball.hitbox, goal2.hitbox)) {
      resetGame(map);
      player1Score++;
    }

    enqueueServerMessage(statusQueue,
                         ServerMessage(map, player1Score, player2Score));

    handleClientMessage(messageQueue, map);
  };

  setupKeybinds(engine, Mode::SERVER);
  engine.setUpdate(update);
}
int main() {

  Object center = {.id = -1,
                   .hitbox = {FIELD_DIMENSION.width / 2.0f,
                              FIELD_DIMENSION.height / 2.0f, 1, 1},
                   .speed = {0, 0},
                   .weight = 1,
                   .color = RED,
                   .shouldMove = false,
                   .shouldCollide = false,
                   .shouldDraw = false};
  Object player1 = {.id = 1,
                    .hitbox = {PLAYER_1_START_POS.x, PLAYER_1_START_POS.y,
                               PLAYER_SIZE, PLAYER_SIZE},
                    .speed = {0, 0},
                    .weight = 1,
                    .color = RED,
                    .shouldMove = true,
                    .shouldCollide = true,
                    .shouldDraw = true,
                    .friction = 0.05f,
                    .elasticity = 1.0f,
                    .isCircle = true};

  Object player2 = {.id = 2,
                    .hitbox = {PLAYER_2_START_POS.x, PLAYER_2_START_POS.y,
                               PLAYER_SIZE, PLAYER_SIZE},
                    .speed = {0, 0},
                    .weight = 1,
                    .color = BLUE,
                    .shouldMove = true,
                    .shouldCollide = true,
                    .shouldDraw = true,
                    .friction = 0.05f,
                    .elasticity = 1.0f,
                    .isCircle = true};

  Object ball = {
      .id = 0,
      .hitbox = {BALL_START_POS.x, BALL_START_POS.y, BALL_SIZE, BALL_SIZE},
      .speed = {0, 0},
      .weight = 0.5,
      .color = WHITE,
      .shouldMove = true,
      .shouldCollide = true,
      .shouldDraw = true,
      .elasticity = 1.0f,
      .isCircle = true};

  Object goal1 = {.id = -1,
                  .hitbox = {GOAL_1_POS.x, GOAL_1_POS.y,
                             (float)GOAL_DIMENSION.width,
                             (float)GOAL_DIMENSION.height},
                  .speed = {0, 0},
                  .weight = 1,
                  .color = BLACK,
                  .shouldMove = false,
                  .shouldCollide = false,
                  .shouldDraw = true};

  Object goal2 = {.id = -2,
                  .hitbox = {GOAL_2_POS.x, GOAL_2_POS.y,
                             (float)GOAL_DIMENSION.width,
                             (float)GOAL_DIMENSION.height},
                  .speed = {0, 0},
                  .weight = 1,
                  .color = BLACK,
                  .shouldMove = false,
                  .shouldCollide = false,
                  .shouldDraw = true};

  Map map(FIELD_DIMENSION, FIELD_COLOR);
  map.addObject(center);
  map.addObject(player1);
  map.addObject(player2);
  map.addObject(ball);
  map.addObject(goal1);
  map.addObject(goal2);

  auto update = [](Map &map, float deltaTime) {
    auto &ball = map.objects[3];
    auto &player1 = map.objects[1];
    auto &player2 = map.objects[2];
    auto &goal1 = map.objects[4];
    auto &goal2 = map.objects[5];
    if (CheckCollisionRecs(ball.hitbox, goal1.hitbox)) {
      resetGame(map);
      player2Score++;
    }
    if (CheckCollisionRecs(ball.hitbox, goal2.hitbox)) {
      resetGame(map);
      player1Score++;
    }
  };

  Renderer renderer(WINDOW_DIMENSION, "Football", 60);
  Engine engine;
  userText[0] = '\0';
  auto showScore = [&engine](Map &map, float deltaTime) {
    // Draw the field lines
    // Center line
    DrawLine(0, FIELD_DIMENSION.height / 2.0f, FIELD_DIMENSION.width,
             FIELD_DIMENSION.height / 2.0f, WHITE);
    // Center circle
    DrawCircleLines(FIELD_DIMENSION.width / 2.0f, FIELD_DIMENSION.height / 2.0f,
                    100, WHITE);
    // Draw goals area
    DrawRectangleLines(GOAL_1_POS.x, GOAL_1_POS.y, GOAL_DIMENSION.width,
                       GOAL_DIMENSION.height + 100.0f, WHITE);
    DrawRectangleLines(GOAL_2_POS.x, GOAL_2_POS.y - 100.0f,
                       GOAL_DIMENSION.width, GOAL_DIMENSION.height + 100.0f,
                       WHITE);
    // Draw the scores
    DrawText(std::to_string(player1Score).c_str(), 100, 100, 100, RED);
    DrawText(std::to_string(player2Score).c_str(), 100, 700, 100, BLUE);
    //  Draw the text box
    GuiTextBox(IP_FIELD_RECT, userText, 100, editText);
    GuiButton(CONNECT_BUTTON_RECT, "connect");
    int startServerButton = GuiButton(START_SERVER_BUTTON_RECT, "start Server");
    if (startServerButton == 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto socket = serverStablishConnection();
      if (!socket.has_value()) {
        printf("Error stablishing the connection\n");
        return;
      }
      enterServerMode(socket.value(), engine);
    }
    // Draw the FPS on the right top corner
    DrawText(std::to_string(GetFPS()).c_str(), WINDOW_DIMENSION.width - 200, 0,
             20, WHITE);
  };

  engine.renderer = renderer;
  engine.map = map;
  setupKeybinds(engine, Mode::STANDALONE);
  engine.setUpdate(update);
  engine.setPreDraw(showScore);

  engine.start();

  return 0;
}
