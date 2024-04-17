#pragma once
#include "common.hpp"
#include "message.hpp"
#include <arpa/inet.h>
#include <optional>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

std::optional<int> clientStablishConnection(const char *ip);
std::optional<int> serverStablishConnection();
void enqueueServerMessage(ServerMessageQueuePtr messageQueue,
                          ServerMessage message);
void sendServerMessage(int sock, ServerMessageQueuePtr messageQueue);
void receiveServerMessage(int sock, ServerMessageQueuePtr statusQueue);
void enqueueClientMessage(ClientMessageQueuePtr messageQueue,
                          ClientMessage message);
void sendClientMessages(int sock, ClientMessageQueuePtr messageQueue);
void sendClientMessage(int sock, ClientMessage message);
void receiveClientMessage(int sock, ClientMessageQueuePtr messageQueue);
void handleClientMessage(ClientMessageQueuePtr messageQueue, Map &map);
