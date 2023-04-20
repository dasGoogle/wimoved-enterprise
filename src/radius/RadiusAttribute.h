//
// Created by aarons on 19.04.23.
//

#ifndef WIMOVED_RADIUS_RADIUSATTRIBUTE_H
#define WIMOVED_RADIUS_RADIUSATTRIBUTE_H

#include <cstdint>
#include <cstring>

namespace radius {
class RadiusAttribute {
   public:
    RadiusAttribute(uint8_t *data, size_t length);
    RadiusAttribute(unsigned char type, unsigned char length, unsigned char *value);
    void encode(uint8_t *buffer, size_t buflen);
    uint8_t type;
    uint8_t length;
    // TODO: Dynamic length
    uint8_t value[256] = {0};
};
}  // namespace radius

#endif  // WIMOVED_RADIUS_RADIUSATTRIBUTE_H
