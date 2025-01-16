#include "irTools.hpp"

namespace coco {

/// @brief Check the correct duration of an interval between two similar edges, e.g. two starts of a mark. This should
/// be quite precise
/// @param t time to check in 50μs units
/// @param time expected time in microseconds
/// @return true if duration is correct within tolerance
inline bool checkInterval(int t, Microseconds<> time) {
    int tMin = time.value * 1180 >> 16; // -10%
    int tMax = (time.value * 1442 >> 16) + 1; // + 10%

    return t >= tMin && t <= tMax;
}

/// @brief Check the correct duration of a mark. This is imprecise as the IR receiver may take longer to detect the
/// presence of the mark than to detect the absence of the mark if the signal is weak
/// @param t time to check in 50μs units
/// @param time expected time in microseconds
/// @return true if duration is correct within tolerance
inline bool checkMark(int t, Microseconds<> time) {
    int tMin = (time.value - 100) * 1114 >> 16; // -100μs -15%
    int tMax = (time.value * 1507 >> 16) + 1; // +15%

    return t >= tMin && t <= tMax;
}

bool checkLeader(const uint8_t* times, Microseconds<> markTime, Microseconds<> spaceTime) {
    // check for packet start
    if (times[0] != 0)
        return false;

    // check mark time
    int mt = times[1];
    if (!checkMark(mt, markTime))
        return false;

    // check total time (sum of mark and space, this way the time between similar edges is checked which is more precise than between different edges)
    int t = mt + times[2];
    return checkInterval(t, markTime + spaceTime);
}

bool decodeVariableLength(const uint8_t* times, Microseconds<> markTime, Microseconds<> spaceTime0, Microseconds<> spaceTime1, uint8_t *data, int count) {
    //int timesIndex = 0;
    int value = 0;
    for (int i = 0; i < count; ++i) {
        // check mark time
        int mt = *(times++);
        if (!checkMark(mt, markTime))
            return false;

        // total time
        int t = mt + *(times++);

        // short or long space determines bit value
        value <<= 1;
        if (checkInterval(t, markTime + spaceTime0))
            ;
        else if (checkInterval(t, markTime + spaceTime1))
            value |= 1;
        else
            return false;

        if ((i & 7) == 7) {
            *data = value;
            value = 0;
            ++data;
        }
    }
    if ((count & 7) != 0)
        *data = value;
    return true;
}

bool decodeVariableMark(const uint8_t* times, Microseconds<> markTime0, Microseconds<> markTime1, uint8_t *data, int count) {
    int value = 0;
    for (int i = 0; i < count; ++i) {
        // mark time
        int mt = *(times++);

        ++times;

        // total time
        //int t = mt + *(times++);

        // check total time
        //if (!checkInterval(t, totalTime))
        //    return false;

        // mark time determines bit value
        value <<= 1;
        if (checkMark(mt, markTime0))
            ;
        else if (checkMark(mt, markTime1))
            value |= 1;
        else
            return false;

        if ((i & 7) == 7) {
            *data = value;
            value = 0;
            ++data;
        }
    }
    if ((count & 7) != 0)
        *data = value;
    return true;
}

bool ManchesterDecoder::decode(Microseconds<> markTime, Microseconds<> spaceTime, uint8_t *data, int count) {
    /*
        Synchronisation happens only at start of mark (depicted as high level, actual IR receiver may output low level
        when a mark is detected). This way the time between two synchronisation points can be measured quite precise
        while the duration of a mark is checked with more error margin due to different response times of the IR
        receiver for detecting mark and space.
        4 possible cases for last and current bit, syncronization points are marked with ^

             0         0
        |     ____|     ____|
        |____|    |____|    |
             ^         ^

             0         1
        |     ____|____     |
        |____|    |    |____|
             ^

             1         0
        |____     |     ____|
        |    |____|____|    |
                       ^

             1         1
        |____     |____     |
        |    |____|    |____|
                  ^
    */

    int value = 0;
    for (int i = 0; i < count; ++i) {
        value <<= 1;
        bool bit = false;
        if (!this->lastBit) {
            // last bit was 0

            // time until rising edge (sync edge) for bit 0
            auto time0 =  this->syncTime + spaceTime;

            // mark time until falling edge (at start of interval for 0, in the middle of interval for 1)
            if (this->times == this->end)
                return false;
            int mt = *(this->times++);

            // time until next rising edge (sync edge)
            if (this->times == this->end)
                return false;
            int st = *this->times;
            int t = mt + st;

            // time of next rising edge determines bit value
            if (checkInterval(t, time0)) {
                // bit 0

                // consume edge which is in the middle of the interval of the bit 0
                ++this->times;

                // check mark time
                auto markTime0 = this->syncTime;
                if (!checkMark(mt, markTime0))
                    return false;

                // set time since last sync edge (is markTime as bit 0 ends with a mark)
                this->syncTime = markTime;
                this->t = 0;
            } else {
                // bit 1

                // check mark time
                auto markTime1 = this->syncTime + spaceTime;
                if (!checkMark(mt, markTime1))
                    return false;

                // no sync edge, therefore add mark and space time to existing sync time
                this->syncTime += markTime + spaceTime;
                this->t = mt;

                bit = 1;
            }
        } else {
            // last bit was 1

            // time until rising edge (sync edge) for bit 0
            auto time0 = this->syncTime + spaceTime;

            // time until rising edge (sync edge) for bit 1
            auto time1 =  this->syncTime;

            // space time until rising edge (sync edge, at start of interval for 1, in the middle of interval for 0)
            if (this->times == this->end)
                return false;
            int st = *(this->times++);

            int t = this->t + st;

            // time of next rising edge determines bit value
            if (checkInterval(t, time0)) {
                // bit 0

                // set time since last sync edge (is markTime as bit 0 ends with a mark)
                this->syncTime = markTime;
                this->t = 0;
            } else if (checkInterval(t, time1)) {
                // bit 1

                // mark time
                if (this->times == this->end)
                    return false;
                int mt = *(this->times++);

                // check mark time
                auto markTime1 = markTime;
                if (!checkMark(mt, markTime1))
                    return false;

                // set time since last sync edge (is whole interval)
                this->syncTime = markTime + spaceTime;
                this->t = mt;

                bit = 1;;
            } else {
                return false;
            }
        }
        this->lastBit = bit;
        value |= int(bit);

        if ((i & 7) == 7) {
            *data = value;
            value = 0;
            ++data;
        }
    }
    if ((count & 7) != 0)
        *data = value;
    return true;
}

} // namespace coco
