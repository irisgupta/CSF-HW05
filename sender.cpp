#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server

  Connection connection;
  connection.connect(server_hostname, server_port);
  if (!connection.is_open()) {
    std::cerr << "Connection Failed\n";
    return 1;
  }

  // TODO: send slogin message

  connection.send(Message(TAG_SLOGIN, username));
  Message message;
  connection.receive(message);
  if (message.tag == TAG_ERR) {
      std::cerr << message.data << "\n";
      return 1;
    }
  
  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  while (1) { 
   std::string str;
   getline(std::cin, str);
   std::string payload;
   std::string command;
   std::stringstream ss;
   ss << str;
   ss >> command; 
  if (command.compare("/join") == 0) {
    ss >> payload;
    conn.send(Message(TAG_JOIN, payload));
    conn.receive(ok_msg);
    if (ok_msg.tag == TAG_ERR) {
      std::cerr << ok_msg.data  << "\n";
    }
  } else if (command.compare("/leave") == 0) {
      conn.send(Message(TAG_LEAVE, ""));
      conn.receive(ok_msg);
      if (ok_msg.tag == TAG_ERR) {
        std::cerr << payload << "\n";
      }
  } else if (command.compare("/quit") == 0) {
      conn.send(Message(TAG_QUIT, ""));
      conn.receive(ok_msg);
    if (ok_msg.tag == TAG_ERR) {
        std::cerr << "bye" << "\n";
      }
      return 0;
  } else { 
      conn.send(Message(TAG_SENDALL, str));
      conn.receive(ok_msg);
      if (ok_msg.tag == TAG_ERR) {
	std::cerr << payload << "\n";
      }
    }
  }
  conn.close();
  return 0;

  return 0;
}