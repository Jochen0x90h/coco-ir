#pragma once

#include <coco/Array.hpp>
#include <coco/Time.hpp>
#include <cstdint>


namespace coco {

// also see https://github.com/crankyoldgit/IRremoteESP8266

/// @brief Check the leader consisting of a mark and a space of defined length (not merged with following data)
/// @param times sampled transition times in 50μs resolution, at least 3 values
/// @param markTime mark time
/// @param spaceTime space time
/// @return true if leader is as expected
bool checkLeader(const uint8_t* times, Microseconds<> markTime, Microseconds<> spaceTime);

/// @brief Decode data with variable bit length
/// @param times sampled transition times in 50μs resolution, at least count * 2 values
/// @param markTime mark time
/// @param spaceTime0 space time for bit 0
/// @param spaceTime1 space time for bit 1
/// @param data output data, at least count values
/// @param count number of bits to decode
/// @return true if successful
bool decodeVariableLength(const uint8_t* times, Microseconds<> markTime, Microseconds<> spaceTime0, Microseconds<> spaceTime1, uint8_t *data, int count);

/// @brief Decode data with variable mark length
/// @param times sampled transition times in 50μs resolution, at least count * 2 values
/// @param markTime0 mark time for bit 0
/// @param markTime1 mark time for bit 1
/// @param data output data, at least count values
/// @param count number of bits to decode
/// @return true if successful
bool decodeVariableMark(const uint8_t* times, Microseconds<> markTime0, Microseconds<> markTime1, uint8_t *data, int count);

/// @brief Decode manchester. Needs a state to measure the time since the last start of a mark to syncrhonize
///
class ManchesterDecoder {
public:
    ManchesterDecoder(const uint8_t* times, int count) : times(times), end(times + count) {}
    bool decode(Microseconds<> markTime, Microseconds<> spaceTime, uint8_t *data, int count);

    const uint8_t* times;
    const uint8_t* end;

    bool lastBit = false;
    Microseconds<> syncTime = {};
    int t = 0;
};

} // namespace coco
