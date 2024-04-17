#include "footballGame/network.hpp"
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

void enqueueServerMessage(
    ServerMessageQueuePtr messageQueue,
    ServerMessage message) {
  messageQueue->push(message);
}

void sendServerMessage(
    int sock, ServerMessageQueuePtr messageQueue) {
  while (true) {
    if (!messageQueue->empty()) {
      ServerMessage message = messageQueue->front();
      messageQueue->pop();
      std::vector<char> serialized = message.serialize();
      if (serialized.size() > 0) {
        int sendServerMessage =
            send(sock, serialized.data(), serialized.size(), 0);
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

void receiveServerMessage(
    int sock, ServerMessageQueuePtr statusQueue) {
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

void enqueueClientMessage(
    ClientMessageQueuePtr messageQueue,
    ClientMessage message) {
  messageQueue->push(message);
}

void sendClientMessages(
    int sock, ClientMessageQueuePtr messageQueue) {
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

void receiveClientMessage(
    int sock, ClientMessageQueuePtr messageQueue) {
  while (true) {
    char buffer[2048] = {0};
    int valread = read(sock, buffer, 2048);
    std::vector<char> message(buffer, buffer + valread);
    ClientMessage messageObj(message);
    messageQueue->push(messageObj);
  }
}

void handleClientMessage(
    ClientMessageQueuePtr messageQueue, Map &map) {
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
