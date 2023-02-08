#include <chrono>
#include <stdexcept>
#include <sstream>
#include "Caller.h"
#include "../Station.h"

const std::string VLAN_ID_PREFIX = "vlan_id=";

ipc::Caller::Caller(const std::string &iface) : socket(iface, std::chrono::seconds(1)) {

}

uint32_t ipc::Caller::vlan_for_station(const std::string &station_mac) {
    std::optional<std::string> result = socket.send_and_receive({"STA", station_mac});
    if (!result.has_value()) {
        throw std::runtime_error("timeout in vlan_for_station");
    }
    std::istringstream stream(result.value());
    std::string line;
    while (std::getline(stream, line)) {
        if (line.rfind(VLAN_ID_PREFIX) == 0) {
            return std::stol(line.substr(VLAN_ID_PREFIX.size(), line.size()));
        }
    }
    throw std::runtime_error("no vlan_id attribute found for station");
}

std::vector<Station> ipc::Caller::connected_stations() {
    std::optional<std::string> result = socket.send_and_receive({"STA-FIRST"});
    if (!result.has_value()) {
        throw std::runtime_error("timeout in connected_stations");
    }
    std::vector<Station> stations{Station(result.value().substr(0, MAC_ADDRESS_LENGTH))};
    return stations;
}
