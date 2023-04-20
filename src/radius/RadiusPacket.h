//
// Created by aarons on 19.04.23.
//

#ifndef WIMOVED_RADIUS_RADIUSPACKET_H
#define WIMOVED_RADIUS_RADIUSPACKET_H

#include <netinet/in.h>
#include <sys/socket.h>

#include <bitset>
#include <vector>

#include "RadiusAttribute.h"
namespace radius {
class RadiusPacket {
   public:
    RadiusPacket(uint8_t *buffer, size_t buffer_length);
    uint16_t length();
    uint8_t code;
    uint8_t identifier;
    uint8_t authenticator[16] = {0};
    std::vector<RadiusAttribute> attributes;
    size_t encode(uint8_t *buffer, size_t buflen);
    void print();

    struct sockaddr_in from;
    struct sockaddr_in to;

    enum Code { ACCESS_REQUEST = 1, ACCESS_ACCEPT = 2, ACCESS_REJECT = 3, ACCESS_CHALLENGE = 11, RESERVED = 255 };

   private:
};
}  // namespace radius

#endif  // WIMOVED_RADIUS_RADIUSPACKET_H
