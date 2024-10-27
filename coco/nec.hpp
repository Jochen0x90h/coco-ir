#pragma once

#include <coco/Array.hpp>
#include <cstdint>


namespace coco {

/// @brief NEC decoder
/// Format: Leader, address, inverse address, command, inverse command
/// https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
namespace nec {

/// @brief Packet
/// Only checks for inverse command as some remote controls use the address bytes differently, but are constant for all keys
struct Packet {
    uint8_t address1;
    uint8_t address2;
    uint8_t command;
};

/// @brief Decode NEC packet
/// @param times timing data
/// @param packet output packet
/// @return true if decoding was successful
bool decode(Array<const uint8_t> times, Packet &packet);

} // namespace nec
} // namespace coco