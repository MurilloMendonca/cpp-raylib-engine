#include <condition_variable>
#include <memory>
#include <optional>
#define GLSL_VERSION 330
#include "engine.hpp"
#include "message.hpp"
#include "raylib.h"
#include "rlgl.h"
#include <arpa/inet.h>
#include <queue>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <thread>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

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
int player1Score = 0;
int player2Score = 0;
bool editText = false;
char userText[100] = {0};

enum class Mode { SERVER, CLIENT, STANDALONE };
enum class Direction { UP, DOWN, LEFT, RIGHT };
enum class ObjectType { BALL = 0, PLAYER1, PLAYER2, GOAL, CENTER };

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
std::optional<int> clientStablishConnection(const char *ip);
std::optional<int> serverStablishConnection();
void enqueueServerMessage(std::shared_ptr<std::queue<ServerMessage>> messageQueue, ServerMessage message);
void sendServerMessage(int sock, std::shared_ptr<std::queue<ServerMessage>> messageQueue);
void receiveServerMessage(int sock, std::shared_ptr<std::queue<ServerMessage>>statusQueue);
void updateMap(std::shared_ptr<std::queue<ServerMessage>> statusQueue, Map &map);
void enqueueClientMessage(std::shared_ptr<std::queue<ClientMessage>> messageQueue, ClientMessage message);
void sendClientMessages(int sock, std::shared_ptr<std::queue<ClientMessage>> messageQueue);
void sendClientMessage(int sock, ClientMessage message);
void receiveClientMessage(int sock, std::shared_ptr<std::queue<ClientMessage>> messageQueue);
void handleClientMessage(std::shared_ptr<std::queue<ClientMessage>>messageQueue, Map &map);

// ==============================
// Connection related functions
// ==============================
std::optional<int> clientStablishConnection(const char *ip) {
  int sock = 0;
  struct sockaddr_in serv_addr;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return {};
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8989);
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return {};
  }
  printf("Connected to server\n");
  return sock;
}

std::optional<int> serverStablishConnection() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8989);
  int result = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  if (result == -1) {
    printf("Error binding the socket\n");
    return {};
  }
  int list = listen(server_fd, 3);
  if (list == -1) {
    printf("Error listening to the socket\n");
    return {};
  }
  printf("Server is listening\n");
  int socket = accept(server_fd, (struct sockaddr *)&address,
                      (socklen_t *)&addrlen); // Accept the connection
  printf("Server accepted the connection\n");
  return socket;
}

void enqueueServerMessage(std::shared_ptr<std::queue<ServerMessage>> messageQueue, ServerMessage message) {
  messageQueue->push(message);
}

void sendServerMessage(int sock, std::shared_ptr<std::queue<ServerMessage>> messageQueue) {
  while (true) {
    if (!messageQueue->empty()) {
      ServerMessage message = messageQueue->front();
      messageQueue->pop();
      std::vector<char> serialized = message.serialize();
      if (serialized.size() > 0) {
        int sendServerMessage = send(sock, serialized.data(), serialized.size(), 0);
        if (sendServerMessage == -1) {
          printf("Error sending message\n");
        }
        if (sendServerMessage == 0) {
          printf("Client disconnected\n");
        }
      }
    }
  }
}

void receiveServerMessage(int sock, std::shared_ptr<std::queue<ServerMessage>>statusQueue) {
  while (true) {
    char buffer[2048] = {0};
    int valread = read(sock, buffer, 2048);
    if (valread == 0) {
      printf("Server disconnected\n");
      return;
    }
    if (valread <= -1) {
      printf("Error reading from server\n");
      return;
    }
    std::vector<char> message(buffer, buffer + valread);
    ServerMessage status(message);
    statusQueue->push(status);
  }
}

void updateMap(std::shared_ptr<std::queue<ServerMessage>>statusQueue, Map &map) {
  if (!statusQueue->empty()) {
    ServerMessage status = statusQueue->front();
    statusQueue->pop();
    status.updateMap(map);
    player1Score = status.player1Score;
    player2Score = status.player2Score;
  }
}

void enqueueClientMessage(std::shared_ptr<std::queue<ClientMessage>> messageQueue, ClientMessage message) {
  messageQueue->push(message);
}

void sendClientMessages(int sock, std::shared_ptr<std::queue<ClientMessage>> messageQueue) {
  while (true) {
    if (!messageQueue->empty()) {
      ClientMessage message = messageQueue->front();
      messageQueue->pop();
      sendClientMessage(sock, message);
    }
  }
}

void sendClientMessage(int sock, ClientMessage message) {
  std::vector<char> serialized = message.serialize();
  if (serialized.size() > 0) {
    int sendServerMessage = send(sock, serialized.data(), serialized.size(), 0);
    if (sendServerMessage == -1) {
      printf("Error sending message\n");
    }
    if (sendServerMessage == 0) {
      printf("Client disconnected\n");
    }
  }
}

void receiveClientMessage(int sock, std::shared_ptr<std::queue<ClientMessage>>messageQueue) {
  while (true) {
    char buffer[2048] = {0};
    int valread = read(sock, buffer, 2048);
    std::vector<char> message(buffer, buffer + valread);
    ClientMessage messageObj(message);
    messageQueue->push(messageObj);
  }
}

void handleClientMessage(std::shared_ptr<std::queue<ClientMessage>> messageQueue, Map &map) {
  if (!messageQueue->empty()) {
    ClientMessage message = messageQueue->front();
    messageQueue->pop();
    printf("Received message:\n");
    message.print();
    auto &player = map.objects[2];
    player.acceleration.x = message.acceleration_x;
    player.acceleration.y = message.acceleration_y;
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

    enqueueServerMessage(statusQueue, ServerMessage(map, player1Score, player2Score));

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

  userText[0] = '\0';
  auto showScore = [](Map &map, float deltaTime) {
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
    GuiButton(START_SERVER_BUTTON_RECT, "start Server");

    // Draw the FPS on the right top corner
    DrawText(std::to_string(GetFPS()).c_str(), WINDOW_DIMENSION.width - 200, 0,
             20, WHITE);
  };

  Renderer renderer(WINDOW_DIMENSION, "Football", 60);
  Engine engine;
  engine.renderer = renderer;
  engine.map = map;
  setupKeybinds(engine, Mode::STANDALONE);
  engine.setUpdate(update);
  engine.setPreDraw(showScore);

  engine.start();

  return 0;
}
