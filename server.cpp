#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

struct ConnectionData {
  Server *server;
  Connection *connection;
  ConnectionData(Server *server, Connection *connection) : server(server), connection(connection) {}
  ~ConnectionData() {
    delete connection;
  }
};


namespace {

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)

  ConnectionData *data = static_cast<ConnectionData *>(arg);
  std::unique_ptr<ConnectionData> dataPointer(data);

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response

  Message message;
  if (!dataPointer->connection->receive(message)) {
    if (dataPointer->connection->get_last_result() == Connection::INVALID_MSG) {
      dataPointer->connection->send(Message(TAG_ERR, "Invalid message!"));
    }
    return nullptr;
  }
  if (message.tag != TAG_RLOGIN && message.tag != TAG_SLOGIN) {
    dataPointer->connection->send(Message(TAG_ERR, "slogin or rlogin required!"));
    return nullptr;
  }
  if (!dataPointer->connection->send(Message(TAG_OK, "Hello, " + message.data))) {
    return nullptr;
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)

  std::string roomName = "";
  std::string client = "";
  if (message.tag == TAG_SLOGIN) {
    client = "sender";
  }
  
  while (true) {
    if (!dataPointer->connection->receive(message)) {
      if (dataPointer->connection->get_last_result() == Connection::INVALID_MSG) {
        dataPointer->connection->send(Message(TAG_ERR, "Invalid message!"));
      }
      break;
    } else {
      if (client == "sender") {
        roomName = dataPointer->server->sender_interaction(dataPointer->connection, dataPointer->server, message, message.data);
        if (roomName == "quit") {
          return nullptr;
        }
      } else {
        dataPointer->server->receiver_interaction(dataPointer->connection, dataPointer->server, message, message.data);
      }
    }
  }
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////
void Server::receiver_interaction(Connection *connection, Server *server, Message message, std::string username) {
  User member(message.data);
  std::string roomName = "";
  Room *room;
  if (message.tag == TAG_JOIN) {
    roomName = message.data; 
    connection->send(Message(TAG_OK, "Okay!"));
    room = server->find_or_create_room(roomName);
    room->add_member(&member);
  }
  Message *messageOngoing;
  while (true) {
    messageOngoing = member.mqueue.dequeue(); 
    connection->send(*messageOngoing);
  }
  room->remove_member(&member);
}

std::string Server::sender_interaction(Connection *connection, Server *server, Message message, std::string username) {
  Room *room;
  std::string roomName = "";
  while(true) {
    if (message.tag == TAG_JOIN) { 
      room = server->find_or_create_room(message.data);
      roomName = room->get_room_name();
      connection->send(Message(TAG_OK, "Okay!"));
    } else if (message.tag == TAG_QUIT) {
      connection->send(Message(TAG_OK, "Time to quit!"));
      roomName = "quit";
      break;
    } else if (message.tag == TAG_LEAVE) {
      if(roomName == "") {
        connection->send(Message(TAG_ERR, "Invalid room!"));
      } else {
        connection->send(Message(TAG_OK, "Time to leave!"));
        roomName = "";
      }
    } else if (message.tag == TAG_SENDALL) {
      if(roomName == "") { 
        connection->send(Message(TAG_ERR, "Invalid room!"));
      } else {
        room->broadcast_message(username, message.data);
        connection->send(Message(TAG_OK, "Okay!"));
      }
    } else {
      connection->send(Message(TAG_ERR, "Invalid tag!"));
    }
    connection->receive(message);
  }
  return roomName;
}


Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::string port = std::to_string(m_port);
  m_ssock = open_listenfd(port.c_str());
  if (m_ssock > 0) {
    return true;
  }
  else {
    return false;
  }
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true && m_ssock >= 0) {
    int clientfd = accept(m_ssock, nullptr, nullptr);
    if (clientfd < 0) {
      std::cerr << "Connection not accepted!\n";
      return;
    }
    ConnectionData *data = new ConnectionData(new Connection(clientfd), this);
    pthread_t thr_id;
    if (pthread_create(&thr_id, nullptr, worker, static_cast<void *>(data)) != 0) {
      std::cerr << "Error creating thread!\n";
      return;
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  Guard g(m_lock);
  Room *room;
  if (m_rooms.find(room_name) == m_rooms.end()) {
    room = new Room(room_name);
    m_rooms[room_name] = room;
  } else {
    room = (m_rooms.find(room_name))->second;
  }
  return room;
}