/*
 * Implementation of the receiver client
 * CSF Assignment 5
 * Iris Gupta and Eric Wang
 * igupta5@jhu.edu and ewang42@jhu.edu
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

/*
 * Have the receiver client connect to a server, send
 * login and join messages to the server, and await
 * messages from the server in a loop
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
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // Connect to server
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()) {
    std::cerr << "Connection Failed\n";
    return 1;
  }
  
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  conn.send(Message(TAG_RLOGIN, username));
  Message message;
  conn.receive(message);
  if (message.tag == TAG_ERR || message.tag != TAG_OK) {
    std::cerr << message.data << "\n";
    return 1;
  }
  conn.send(Message(TAG_JOIN, room_name));
  conn.receive(message);
  if (message.tag == TAG_ERR || message.tag != TAG_OK) {
    std::cerr << message.data << "\n";
    return 1;
  }
  
  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while	(1) {
    conn.receive(message);

    if (message.tag == TAG_DELIVERY) {
      std::string data = message.data;
      std::size_t colonIndex = data.find(":");
      std::string room = data.substr(0, colonIndex);
      std::string senderAndMessage = data.substr(colonIndex + 1);
      colonIndex = senderAndMessage.find(":");
      std::string sender = senderAndMessage.substr(0, colonIndex);
      std::string senderMessage = senderAndMessage.substr(colonIndex + 1);
      if (room.compare(room_name) == 0) {
        std::cout << sender << ": " << senderMessage << std::endl;
      } else {
        break;
      }
    } else {
      std::cerr << "Could not receive message" << std::endl;
      return 0;
    }

  }
  conn.close();
  return 0;
}
