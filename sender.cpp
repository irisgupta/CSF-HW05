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
   std::string text;
   getline(std::cin, text);
   std::string payload;
   std::string command;
   std::stringstream ss;
   ss << text;
   ss >> command; 
  if (command == "/join") {
    ss >> payload;
    connection.send(Message(TAG_JOIN, payload));
    connnection.receive(message);
    if (message.tag == TAG_ERR) {
      std::cerr << message.data  << "\n";
    }
  } else if (command == "/leave") {
      connection.send(Message(TAG_LEAVE, ""));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
        std::cerr << payload << "\n";
      }
  } else if (command == "/quit") {
      connection.send(Message(TAG_QUIT, ""));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
        std::cerr << "Thank you!" << "\n";
      }
      return 0;
  } else { 
      connection.send(Message(TAG_SENDALL, text));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
	      std::cerr << payload << "\n";
      }
    }
  }
  conn.close();
  return 0;
}