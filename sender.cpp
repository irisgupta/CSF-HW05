/*
 * Implementation of the sender client
 * CSF Assignment 5
 * Iris Gupta and Eric Wang
 * igupta5@jhu.edu and ewang42@jhu.edu
 */

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

/*
 * Have the sender client connect to a server, send
 * a login message to the server, and send multiple
 * commands to the server
 *
 * Parameters:
 *   argc - integer number of command line arguments
 *   argv - character array containing each command line argument
 *
 * Returns:
 *   an integer (either 1 or 0) to represent success/failure of the
 *   program
 */
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
      connection.receive(message);
      if (message.tag == TAG_ERR) {
        std::cerr << message.data << "\n";
      }
    } else if (command == "/leave") {
      connection.send(Message(TAG_LEAVE, ""));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
        std::cerr << message.data << "\n";
      }
    } else if (command == "/quit") {
      connection.send(Message(TAG_QUIT, ""));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
        std::cerr << message.data << "\n";
      }
      return 0;
    } else {
      connection.send(Message(TAG_SENDALL, text));
      connection.receive(message);
      if (message.tag == TAG_ERR) {
	      std::cerr << message.data << "\n";
      }
    }
  }
  connection.close();
  return 0;
}
