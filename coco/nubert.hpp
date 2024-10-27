#pragma once

#include <coco/Array.hpp>
#include <cstdint>


namespace coco {

/// @brief Nubert decoder
/// Format: Start bit, 10 data bits, stop bit
/// https://www.mikrocontroller.net/articles/IRMP#NUBERT
namespace nubert {

/// @brief Decode Nubert packet
/// @param times timing data
/// @param packet output packet
/// @return true if decoding was successful
bool decode(Array<const uint8_t> times, uint16_t &packet);

} // namespace nubert
} // namespace coco
