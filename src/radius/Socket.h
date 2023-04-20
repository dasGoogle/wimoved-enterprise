#ifndef WIMOVED_RADIUS_SOCKET_H
#define WIMOVED_RADIUS_SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <chrono>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "RadiusPacket.h"

namespace radius {

class Socket {
   public:
    explicit Socket(const uint16_t local_port, const std::chrono::milliseconds &timeout);
    ~Socket();
    Socket(const Socket &other) = delete;
    void operator=(const Socket &other) = delete;
    Socket(Socket &&other) noexcept;

    void send(radius::RadiusPacket packet);
    radius::RadiusPacket receive();

   private:
    int sock_fd;
    struct sockaddr_in local;
};
}  // namespace radius

#endif  // WIMOVED_RADIUS_SOCKET_H
