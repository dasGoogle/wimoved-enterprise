#ifndef WIMOVED_CONFIGURATION_H
#define WIMOVED_CONFIGURATION_H

#include <cstdint>
#include <string>
#include <vector>

class ConfigParser;

class Configuration {
   public:
    static Configuration& get_instance() { return INSTANCE; }
    static void reset() { INSTANCE = Configuration{}; }

    ~Configuration() = default;
    Configuration(Configuration const&) = delete;
    Configuration(Configuration&&) = delete;
    Configuration& operator=(Configuration const&) = delete;

    void apply_config_file(const ConfigParser& parser);
    void apply_environment();
    void check_validity() const;

    static const uint32_t DEFAULT_MIN_VNI;
    static const uint32_t DEFAULT_MAX_VNI;
    static const std::string DEFAULT_HAPD_SOCKDIR;
    static const std::string DEFAULT_HAPD_GROUP;
    static const std::string DEFAULT_LOG_PATH;
    static const std::vector<std::string> DEFAULT_SOCKNAMES;
    static const uint32_t DEFAULT_CLEANUP_INTERVAL;
    static const uint16_t DEFAULT_RADIUS_PROXY_PORT;
    static const uint16_t DEFAULT_RADIUS_SERVER_PORT;
    static const std::string DEFAULT_RADIUS_SERVER_IP;

    uint32_t min_vni = DEFAULT_MIN_VNI;
    uint32_t max_vni = DEFAULT_MAX_VNI;
    std::string hapd_sockdir = DEFAULT_HAPD_SOCKDIR;
    std::string hapd_group = DEFAULT_HAPD_GROUP;
    std::string log_path = DEFAULT_LOG_PATH;
    std::vector<std::string> socknames = DEFAULT_SOCKNAMES;
    uint32_t cleanup_interval = DEFAULT_CLEANUP_INTERVAL;

    //Radius Server Configuration
    std::string radius_server_ip = DEFAULT_RADIUS_SERVER_IP;
    uint16_t radius_server_port = DEFAULT_RADIUS_SERVER_PORT;
    uint16_t radius_proxy_port = DEFAULT_RADIUS_PROXY_PORT;

   private:
    Configuration() = default;
    void set_all_available_sockets_if_empty();
    Configuration& operator=(Configuration&&) = default;

    static Configuration INSTANCE;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

};

#include "ConfigParser.h"

#endif  // WIMOVED_CONFIGURATION_H
