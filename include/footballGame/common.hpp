#pragma once

#include "message.hpp"
#include <memory>
#include <queue>
typedef std::queue<ServerMessage> ServerMessageQueue;
typedef std::queue<ClientMessage> ClientMessageQueue;

typedef std::shared_ptr<std::queue<ServerMessage>> ServerMessageQueuePtr;
typedef std::shared_ptr<std::queue<ClientMessage>> ClientMessageQueuePtr;
