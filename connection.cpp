#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuffer, fd); 
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  std::string portString = std::to_string(port);
  m_fd = open_clientfd(hostname, portString); 
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuffer, m_fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if (is_open()) {
    Close(m_fd);
  }
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  if (m_fd < 0 ) {
    return false;
  }
  return true;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (is_open()) {
    Close(m_fd);
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (is_open()) {
    std::stringstream ss; 
    ss << msg.tag << ":" << msg.data << "\n"; 
    std::string message = ss.str(); 
    if (rio_writen(m_fd, message, message.length()) == -1) {
      m_last_result = EOF_OR_ERROR;
      return false;
    }
    m_last_result = SUCCESS;
    return true;
  }
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char store[msg.MAX_LEN];
  int newRead = Rio_readlineb(&m_fdbuffer, store, msg.MAX_LEN);
  if (newRead < 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  std::string result = store;
  if (result[result.length()-1] == '\n') {
  result.erase(result.length()-1);
  }
  int index = 0;
  std::size_t colonLocation = result.find(":");
  if (colonLocation != std::string::npos) {
    msg.tag = result.substr(0,colon);                                                                                                                                                                       
    msg.data = result.substr(colon + 1);                                                                                                                                
    m_last_result = SUCCESS; 
  } else {
    std::cerr << "Error: invalid message format";
    m_last_result = INVALID_MSG;
    return false;
  }
  return true; 
}
