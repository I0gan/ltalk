#pragma once
#include <cstdlib>
#include <string>
#include <string.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "../ltalk.hh"

/*
 * This file is static function set
 *
*/

namespace Ltalk {
namespace Util {

ssize_t ReadData(int fd, void *buffer, size_t length);
ssize_t ReadData(int fd, std::string &in_buffer);

ssize_t WriteData(int fd, void *buffer, size_t length);
ssize_t WriteData(int fd, std::string &out_buffer);
void IgnoreSigpipe(); //avoid server terminate with SIGPIPE signal
bool SetFdNonBlocking(int listen_fd); //set fd as non bloking
void SetFdNoDelay(int fd);
void SetFdNoLinger(int fd);
void ShutDownWriteFd(int fd);

}
}

