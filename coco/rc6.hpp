#pragma once

#include <coco/Array.hpp>
#include <cstdint>


namespace coco {

/// @brief RC6 decoder
/// https://www.sbprojects.net/knowledge/ir/rc6.php
namespace rc6 {

struct Packet {
    uint8_t mode;
    bool trailer;
    uint8_t control;
    uint8_t data;
};

/// @brief Decode RC6 packet
/// @param data timing data
/// @param packet output packet
/// @return true if decoding was successful
bool decode(Array<const uint8_t> data, Packet &packet);

} // namespace rc6
} // namespace coco
