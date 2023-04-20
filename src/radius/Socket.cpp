#include "Socket.h"

#include <grp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>

#include "../Configuration.h"
#include "../TimeoutException.h"
#include "logging/loginit.h"

const static int BUFFER_SIZE = 4096;
const static int MICROSECONDS_PER_SECOND = 1000000;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static std::mutex twister_mutex;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

radius::Socket::Socket(const uint16_t local_port, const std::chrono::milliseconds& timeout) : local() {
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        throw std::runtime_error(std::string("could not create socket: ") + std::strerror(errno));
    }
    auto timeout_usec = std::chrono::microseconds(timeout);
    struct timeval tv {};
    tv.tv_sec = timeout_usec.count() / MICROSECONDS_PER_SECOND;
    tv.tv_usec = timeout_usec.count() % MICROSECONDS_PER_SECOND;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));

    // Configure local socket
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(local_port);

    if (bind(sock_fd, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        close(sock_fd);
        throw std::runtime_error(std::string("could not bind to socket: ") + std::strerror(errno));
    }
}

radius::Socket::~Socket() {
    if (sock_fd == -1) {
        return;
    }
    if (close(sock_fd) == -1) {
        WMLOG(ERROR) << "Could not close socket: " << std::strerror(errno) << "\n";
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void radius::Socket::send(radius::RadiusPacket packet) {
    uint8_t buf[BUFFER_SIZE] = {0};
    size_t len = packet.encode(buf, BUFFER_SIZE);

    ssize_t err = sendto(sock_fd, reinterpret_cast<unsigned char*>(buf), len, MSG_CONFIRM,
                         reinterpret_cast<const struct sockaddr*>(&packet.to), sizeof(packet.to));
    if (err < 0) {
        throw std::runtime_error(std::string("could not send_command to socket: ") + std::strerror(errno));
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
radius::RadiusPacket radius::Socket::receive() {
    uint8_t buf[BUFFER_SIZE] = {0};
    struct sockaddr_in from {};
    while (true) {
        socklen_t addrlen = sizeof(sockaddr_in);
        ssize_t len =
            recvfrom(sock_fd, buf, BUFFER_SIZE, MSG_WAITALL, reinterpret_cast<struct sockaddr*>(&from), &addrlen);
        if (len != -1) {
            RadiusPacket packet(buf, len);
            packet.from = from;
            return packet;
        }
        if (errno == EAGAIN) {
            throw TimeoutException("timeout in recv() from radius socket");
        }
        if (errno != EINTR) {
            throw std::runtime_error(std::string("could not recv from socket: ") + std::strerror(errno));
        }
    }
}
radius::Socket::Socket(radius::Socket&& other) noexcept : sock_fd(other.sock_fd), local(other.local) {
    other.sock_fd = -1;
}
