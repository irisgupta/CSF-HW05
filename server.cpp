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
  ConnectionData(Connection *connection, Server *server) : connection(connection), server(server) {}
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

  ConnectionData *information = static_cast<ConnectionData *>(arg);
  std::unique_ptr<ConnectionData> data(information);

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response

  Message message;
  if (!data->connection->receive(message)) {
    if (data->connection->get_last_result() == Connection::INVALID_MSG) {
      data->connection->send(Message(TAG_ERR, "Invalid message!"));
    }
    return nullptr;
    
  }
  if (message.tag != TAG_RLOGIN && message.tag != TAG_SLOGIN) {
    data->connection->send(Message(TAG_ERR, "slogin or rlogin required!"));
    return nullptr;
  }
  if (!data->connection->send(Message(TAG_OK, "Hello, " + message.data))) {
    return nullptr;
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)

  std::string client = "";
  if (message.tag == TAG_SLOGIN) {
    client = "sender";
  }
  else if (message.tag == TAG_RLOGIN) {
    client = "receiver";
  }
  std::string room = "";
  
  while (true) {
    if (!data->connection->receive(message)) {
      if (data->connection->get_last_result() == Connection::INVALID_MSG) {
        data->connection->send(Message(TAG_ERR, "Invalid message!"));
      }
      break;
    } else {
      if (client == "sender") {
        room = data->server->sender_interaction(data->connection, data->server, message, message.data);
        if (room == "quit") {
          return nullptr;
        }
      } else {
        data->server->receiver_interaction(data->connection, data->server, message, message.data);
      }
    }
  }

  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////
void Server::receiver_interaction(Connection *conn, Server *server, Message msg, std::string username) {
  User member(message.data);
  std::string room = "";
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

std::string Server::sender_interaction(Connection *conn, Server *server, Message msg, std::string username) {
  Room *room;
  std::string roomName = "";
  while(true) {
    if (message.tag == TAG_JOIN) { 
      room = server->find_or_create_room(message.data);
      roomName = room->get_room_name();
      connection->send(Message(TAG_OK, "Okay!"));
    } else if (message.tag == TAG_SENDALL) {
      if(roomName == "") { 
        connection->send(Message(TAG_ERR, "Invalid room!"));
      } else {
        room->broadcast_message(username, message.data);
        connection->send(Message(TAG_OK, "Okay!"));
      }
    } else if (message.tag == TAG_LEAVE) {
      if(roomName == "") {
        connection->send(Message(TAG_ERR, "Invalid room!"));
      } else {
        connection->send(Message(TAG_OK, "Time to leave!"));
        roomName = "";
      }
    } else if (message.tag == TAG_QUIT) {
      connection->send(Message(TAG_OK, "Time to quit!"));
      roomName = "quit";
      break;
    } else {
      conn->send(Message(TAG_ERR, "Invalid tag!"));
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
  return m_ssock;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true && m_ssock >= 0) {
    int clientfd = accept(m_ssock, nullptr, nullptr);
    if (clientfd < 0) {
      std::cerr << "Error with accepting connection!\n";
      return;
    }
    ConnectionData *data = new ConnectionData(new Connection(clientfd), this);
    pthread_t thr_id;
    if (pthread_create(&thr_id, nullptr, worker, static_cast<void *>(data)) != 0) {
      std::cerr << "Could not create thread!\n";
      return;
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary

  Guard g(m_lock);
  Room *room;
  std::string temp = m_rooms.find(room_name);
  if (temp == m_rooms.end()) {
    room = new Room(room_name);
    m_rooms[room_name] = room;
  } else {
    room = temp->second;
  }
  return room;
}
