#include "nubert.hpp"
#include "irTools.hpp"


namespace coco {
namespace nubert {

/*
    Examples
    Celoxon screen remote control
    Up: 0,25,8,25,8,25,8,25,8,8,24,8,25,8,25,8,25,8,25,8,25,8,25,25
    Center: 0,25,8,25,7,25,8,25,8,8,25,9,24,9,24,8,25,8,25,8,25,25,8,8
    Down: 0,25,8,24,8,25,7,25,7,8,25,9,24,8,25,8,25,8,25,25,8,8,25,8
*/

bool decode(Array<const uint8_t> times, uint16_t &packet) {
    // check size and start of packet
    if (times.size() != 24 || times[0] != 0)
        return false;

    // data (MSB first?)
    uint8_t data[2];
    if (!decodeVariableMark(times.data() + 1, 500us, 1340us, data, 12)) // overall length is 1688us
        return false;

    packet = (data[0] << 4) | data[1];

    return true;
}

} // namespace nubert
} // namespace coco
