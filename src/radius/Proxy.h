//
// Created by aarons on 20.04.23.
//

#include <netinet/in.h>
#include <sys/socket.h>

#include <future>
#include <unordered_map>

#include "Socket.h"

#ifndef WIMOVED_PROXY_H
#define WIMOVED_PROXY_H

namespace radius {

class Proxy {
   public:
    Proxy();
    void loop(const std::future<void>& future);
    uint32_t vni_for_user(const std::string& user);

   private:
    struct sockaddr_in radius_server_address;
    struct sockaddr_in last_ap_address;
    radius::Socket socket;
    std::unordered_map<std::string, uint32_t> vni_map;
    void extract_vni_from_packet(RadiusPacket& packet);
    std::mutex mutex;
};
}  // namespace radius

#endif  // WIMOVED_PROXY_H
