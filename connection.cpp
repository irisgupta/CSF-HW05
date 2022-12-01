/*
 * Functions for the connection class to facilitate client/server interaction
 * CSF Assignment 5
 * Iris Gupta and Eric Wang
 * igupta5@jhu.edu and ewang42@jhu.edu
 */

#include <sstream>
#include <cctype>
#include <cassert>
#include <iostream>
#include "csapp.h"
#include "message.h"
#include "connection.h"

/*
 * Default constructor of the Connection class
 */
Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

/*
 * Non-default constructor of the Connection class
 *
 * Parameters:
 *   fd - int file descriptor
 */
Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd);
}

/*
 * Connect to a server given a hostname and port
 *
 * Parameters:
 *   hostname - reference to string hostname
 *   port - integer representing port number
 */
void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  char * portString = (char *)(std::to_string(port)).c_str();
  char * hostnameString = (char *) hostname.c_str();
  m_fd = open_clientfd(hostnameString, portString);
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

/*
 * Destructor for the class Connection
 */
Connection::~Connection() {
  // TODO: close the socket if it is open
  if (is_open()) {
    Close(m_fd);
  }
}

/*
 * Determine whether or not a connection is open
 *
 * Returns:
 *   boolean to represent if the connection is open or not
 */
bool Connection::is_open() const {
  // TODO: return true if the connection is open
  if (m_fd < 0) {
    return false;
  }
  return true;
}

/*
 * Close a connection
 */
void Connection::close() {
  // TODO: close the connection if it is open
  if (is_open()) {
    Close(m_fd);
  }
}

/*
 * Send a message via a connection
 *
 * Parameters:
 *   msg - reference to a Message
 *
 * Returns:
 *   boolean to indicate if a connection send is successful
 */
bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (is_open()) {
    std::stringstream ss; 
    ss << msg.tag << ":" << msg.data << "\n"; 
    std::string message = ss.str(); 
    char * messageString = (char *) message.c_str();
    if (rio_writen(m_fd, messageString, message.length()) == -1) {
      m_last_result = EOF_OR_ERROR;
      return false;
    }
    m_last_result = SUCCESS;
    return true;
  }
  return false;
}

/*
 * Receive a message via a connection
 *
 * Parameters:
 *   msg - reference to a Message
 *
 * Returns:
 *   boolean to indicate if a connection receive is successful
 */
bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char temp[msg.MAX_LEN];
  int newRead = rio_readlineb(&m_fdbuf, temp, msg.MAX_LEN);
  if (newRead < 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  std::string message = temp;
  if (message[message.length()-1] == '\n') {
    message = message.substr(0, message.length()-1);
  }
  std::size_t colonIndex = message.find(":");
  if (colonIndex != std::string::npos) {
    msg.tag = message.substr(0,colonIndex);
    msg.data = message.substr(colonIndex + 1);
    m_last_result = SUCCESS; 
  } else {
    std::cerr << "Error: invalid message format";
    m_last_result = INVALID_MSG;
    return false;
  }
  return true; 
}