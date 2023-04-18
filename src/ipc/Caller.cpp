#include "Caller.h"

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "../Configuration.h"
#include "../Station.h"
#include "logging/loginit.h"

const std::string VLAN_ID_PREFIX = "vlan_id=";
const std::string USERNAME_PREFIX = "dot1xAuthSessionUserName=";

uint32_t ipc::Caller::vlan_for_station(const Station &station) {
    Socket &socket = get_socket(station.sockname);
    std::istringstream stream(socket.send_and_receive({"STA", station.mac.string()}));
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind(VLAN_ID_PREFIX) == 0) {
            return std::stol(line.substr(VLAN_ID_PREFIX.size(), line.size()));
        }
    }
    throw std::runtime_error("no vlan_id attribute found for station");
}

std::string ipc::Caller::user_for_station(const Station &station) {
    Socket &socket = get_socket(station.sockname);
    std::istringstream stream(socket.send_and_receive({"STA", station.mac.string()}));
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind(USERNAME_PREFIX) == 0) {
            return line.substr(USERNAME_PREFIX.size(), line.size());
        }
    }
    throw std::runtime_error("no vlan_id attribute found for station");
}

std::vector<Station> ipc::Caller::connected_stations() {
    std::vector<std::string> socknames = Configuration::get_instance().socknames;
    std::vector<Station> stations;
    for (const auto &sockname : socknames) {
        Socket &socket = get_socket(sockname);
        std::string ipc_result = socket.send_and_receive({"STA-FIRST"});
        while (!ipc_result.empty()) {
            if (ipc_result != "FAIL\n" && ipc_result.length() >= MAC_ADDRESS_LENGTH) {
                stations.emplace_back(sockname, MacAddress(ipc_result.substr(0, MAC_ADDRESS_LENGTH)));
                stations.back().user = user_for_station(stations.back());
            }
            ipc_result = socket.send_and_receive({"STA-NEXT", stations[stations.size() - 1].mac.string()});
        }
    }
    return stations;
}

void ipc::Caller::deauth_station(const Station &station) {
    Socket &socket = get_socket(station.sockname);
    std::string result = socket.send_and_receive({"DEAUTHENTICATE", station.mac.string()});
    if (result != Socket::HOSTAPD_OK) {
        WMLOG(WARNING) << "Did not receive OK on DEAUTH request: " << result;
    }
}
ipc::Socket &ipc::Caller::get_socket(const std::string &name) {
    if (sockets.find(name) == sockets.end()) {
        sockets.emplace(name, Socket{std::chrono::seconds(1), name});
    }
    return sockets.at(name);
}
