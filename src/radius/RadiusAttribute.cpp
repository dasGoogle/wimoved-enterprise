//
// Created by aarons on 19.04.23.
//

#include "RadiusAttribute.h"

#include <cstring>

radius::RadiusAttribute::RadiusAttribute(uint8_t *buffer, size_t buflen) {
    this->type = buffer[0];
    this->length = buffer[1];
    if (length > buflen) {
        throw "Buffer too small";
    }
    memcpy(this->value, buffer + 2, length - 2);
}

void radius::RadiusAttribute::encode(uint8_t *buffer, size_t buflen) {
    if (length > buflen) {
        throw "Buffer too small";
    }
    buffer[0] = type;
    buffer[1] = length;
    memcpy(buffer + 2, value, length - 2);
}
