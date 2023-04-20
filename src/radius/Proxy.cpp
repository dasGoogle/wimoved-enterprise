//
// Created by aarons on 20.04.23.
//

#include "Proxy.h"

#include <arpa/inet.h>

#include <future>

#include "../../vendor/logging/loginit.h"
#include "../Configuration.h"

static std::string LOCAL_IP = "127.0.0.1";
static uint8_t TUNNEL_PRIVATE_GROUP_ID = 81;
static uint8_t USER_NAME = 1;
static uint8_t TUNNEL_TYPE = 64;
static uint8_t TUNNEL_PROTO_VXLAN = 22;

radius::Proxy::Proxy() : socket(Configuration::get_instance().radius_proxy_port, std::chrono::milliseconds(10000)) {
    Configuration& config = Configuration::get_instance();
    // Configure radius server address
    radius_server_address.sin_family = AF_INET;
    radius_server_address.sin_port = htons(config.radius_server_port);
    radius_server_address.sin_addr.s_addr = inet_addr(config.radius_server_ip.c_str());
}

void radius::Proxy::loop(const std::future<void>& future) {
    WMLOG(DEBUG) << "Starting proxy loop";
    while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        try {
            auto packet = socket.receive();
            std::string from_ip = inet_ntoa(packet.from.sin_addr);
            uint16_t port = ntohs(packet.from.sin_port);
            // packet.print();
            if (port == Configuration::get_instance().radius_server_port) {
                // Packet from radius server
                extract_vni_from_packet(packet);
                WMLOG(DEBUG) << "Sending packet to AP, Code: " << std::to_string(packet.code)
                             << " id: " << std::to_string(packet.identifier) << " from " << port;
                packet.to = last_ap_address;
            } else {
                // Packet from AP
                WMLOG(DEBUG) << "Sending packet to RADIUS Server, Code: " << std::to_string(packet.code)
                             << " id: " << std::to_string(packet.identifier) << " from " << port;
                packet.to = radius_server_address;
                last_ap_address = packet.from;
            }
            socket.send(packet);
        } catch (const std::exception& e) {
            // The timeout has been triggered. We do not care.
            WMLOG(DEBUG) << std::string("Timeout triggered") + e.what();
        }
    }
}

void radius::Proxy::extract_vni_from_packet(radius::RadiusPacket& packet) {
    std::string vni_string;
    std::string user;
    uint32_t tunnel_type = 0;
    for (auto& attr : packet.attributes) {
        if (attr.type == TUNNEL_PRIVATE_GROUP_ID) {
            vni_string = reinterpret_cast<char*>(attr.value);
        }
        if (attr.type == USER_NAME) {
            user = reinterpret_cast<char*>(attr.value);
        }
        if (attr.type == TUNNEL_TYPE) {
            tunnel_type = attr.value[0] << 24 | attr.value[1] << 16 | attr.value[2] << 8 | attr.value[3];
        }
    }
    if (!vni_string.empty() && !user.empty() && tunnel_type == TUNNEL_PROTO_VXLAN) {
        uint32_t vni = std::stoi(vni_string);
        {
            std::lock_guard lock(mutex);
            vni_map[user] = vni;
        }
    }
}

uint32_t radius::Proxy::vni_for_user(const std::string& user) {
    {
        std::lock_guard lock(mutex);
        if (!vni_map.contains(user)) {
            throw std::runtime_error("No VNI found for user " + user);
        }
        return vni_map[user];
    }
}

radius::Proxy& radius::Proxy::get_instance() {
    static Proxy instance;
    return instance;
}