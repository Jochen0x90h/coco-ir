#include "nec.hpp"


namespace coco {
namespace nec {

/*
    Examples
    Näve Light remote control
    0: 0,181,88,12,32,12,10,12,10,12,10,12,10,12,9,12,32,12,9,12,9,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,9,12,10,12,32,12,9,12,32,12,9,12,9,12,9,12,32,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12
    1: 0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,32,12,10,12,32,12,10,12,32,12,10,12,10,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12
    Star: 0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,32,12,10,12,32,12
    +: 0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,10,12,9,12,32,12,9,12,10,12,10,12,32,12,10,12,32,12,32,12,10,12,32,12,32,12,32,12
    -: 0,181,88,12,32,12,10,12,10,12,9,12,10,12,9,12,32,12,9,12,9,12,32,12,10,12,32,12,9,12,32,12,32,12,32,12,32,12,32,12,9,12,9,12,32,12,9,12,10,12,10,12,9,12,10,12,32,12,32,12,9,12,32,12,32,12,32,12
*/

// lower/upper limit for 1125μs (0)
constexpr int L1125 = 20;
constexpr int U1125 = 25;

// lower/upper limit for 2250μs (1)
constexpr int L2250 = 40;
constexpr int U2250 = 50;


bool decode(Array<const uint8_t> data, Packet &packet) {
    if (data.size() < 1 + 2 + 2 * 32)
        return false;

    // leader (9ms 4.5ms)
    if (data[1] < 162 || data[1] > 198)
        return false;
    if (data[2] < 81 || data[2] > 99)
        return false;

    int index = 3;
    uint32_t value = 0;
    for (int i = 0; i < 32; ++i) {
        int t = data[index] + data[index + 1];
        index += 2;

        // short or long space determines bit value
        value <<= 1;
        if (t >= L1125 && t <= U1125)
            ;
        else if (t >= L2250 && t <= U2250)
            value |= 1;
        else
            return false;
    }

    // check if last byte is inverse of command
    if (((value ^ (value >> 8)) & 0xff) != 0xff)
        return false;

    packet.address1 = value >> 24;
    packet.address2 = value >> 16;
    packet.command = value >> 8;
    return true;
}

} // namespace nec
} // namespace coco
