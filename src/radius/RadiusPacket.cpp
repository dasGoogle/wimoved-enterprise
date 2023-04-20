//
// Created by aarons on 19.04.23.
//

#include "RadiusPacket.h"

#include <iostream>

const size_t RADIUS_AUTHENTICATOR_LENGTH = 16;
const size_t RADIUS_LENGTH_LENGTH = 2;
const size_t RADIUS_CODE_LENGTH = 1;
const size_t RADIUS_IDENTIFIER_LENGTH = 1;
const size_t RADIUS_HEADER_LENGTH =
    RADIUS_AUTHENTICATOR_LENGTH + RADIUS_LENGTH_LENGTH + RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH;

radius::RadiusPacket::RadiusPacket(uint8_t *buffer, size_t buffer_length) {
    this->code = buffer[0];
    this->identifier = buffer[RADIUS_CODE_LENGTH];
    uint16_t length = buffer[RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH] << 8 |
                      buffer[RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH + 1];
    memcpy(this->authenticator, buffer + RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH + RADIUS_LENGTH_LENGTH,
           RADIUS_AUTHENTICATOR_LENGTH);

    if (length > buffer_length) {
        throw "Buffer too small";
    }
    for (size_t i = RADIUS_HEADER_LENGTH; i < length;) {
        attributes.emplace_back(buffer + i, length - i);
        i += attributes.back().length;
    }
}

uint16_t radius::RadiusPacket::length() {
    uint16_t length = RADIUS_HEADER_LENGTH;
    for (auto &attribute : attributes) {
        length += attribute.length;
    }
    return length;
}

size_t radius::RadiusPacket::encode(uint8_t *buffer, size_t buflen) {
    size_t packet_length = length();
    if (packet_length > buflen) {
        throw "Buffer too small";
    }
    buffer[0] = code;
    buffer[RADIUS_CODE_LENGTH] = identifier;
    buffer[RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH] = (length() >> 8) & 0xFF;
    buffer[RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH + 1] = length() & 0xFF;

    memcpy(buffer + RADIUS_CODE_LENGTH + RADIUS_IDENTIFIER_LENGTH + RADIUS_LENGTH_LENGTH, authenticator,
           RADIUS_AUTHENTICATOR_LENGTH);
    size_t offset = RADIUS_HEADER_LENGTH;
    for (auto &attribute : attributes) {
        attribute.encode(buffer + offset, buflen - offset);
        offset += attribute.length;
    }
    return packet_length;
}

void radius::RadiusPacket::print() {
    std::cout << "===== RADIUS Packet =====" << std::endl;
    std::cout << "Type: " << static_cast<int16_t>(code) << std::endl;
    std::cout << "Size: " << length() << std::endl;
    std::cout << "Identifier: " << static_cast<int16_t>(identifier) << std::endl;
    std::cout << "Authenticator: ";
    for (size_t j = 0; j < RADIUS_AUTHENTICATOR_LENGTH; j++) printf("%02X", authenticator[j]);
    std::cout << std::endl;
    std::cout << "Attributes:" << std::endl;
    for (auto &attr : attributes) {
        std::cout << "    Type: " << static_cast<int16_t>(attr.type) << std::endl;
        std::cout << "        Length: " << static_cast<uint16_t>(attr.length) << std::endl;
        std::cout << "        Value: " << attr.value << " (";
        for (size_t j = 0; j < attr.length - 2; j++) printf("%02X", attr.value[j]);
        std::cout << ")" << std::endl;
    }
}