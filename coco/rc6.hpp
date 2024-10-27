#pragma once

#include <coco/Array.hpp>
#include <cstdint>


namespace coco {

/// @brief RC6 decoder
/// Format: Leader, start bit, 3 mode bits, trailer bit, 16 data bits
/// https://www.sbprojects.net/knowledge/ir/rc6.php
namespace rc6 {

struct Packet {
    uint8_t mode;
    bool trailer;
    uint8_t control;
    uint8_t data;

    bool operator ==(const Packet &other) const {
        return this->mode == other.mode
            && this->trailer == other.trailer
            && this->control == other.control
            && this->data == other.data;
    }
};

/// @brief Decode RC6 packet
/// @param times timing data
/// @param packet output packet
/// @return true if decoding was successful
bool decode(Array<const uint8_t> times, Packet &packet);

} // namespace rc6
} // namespace coco
