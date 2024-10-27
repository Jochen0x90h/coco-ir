#include <coco/debug.hpp>
#include <coco/convert.hpp>
#include <coco/StreamOperators.hpp>
#include <coco/nec.hpp>
#include <coco/nubert.hpp>
#include <coco/rc6.hpp>
#include <IrReceiverTest.hpp>


using namespace coco;

/*
    https://mc.mikrocontroller.com/de/IR-Protokolle.php
    https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/IRrecv.cpp#L1568

    Celoxon
    Up: 0 12 4 13 3 12 4 12 4 4 12 4 12 4 12 4 12 4 12 4 12 4 12 12
    Center: 0 12 4 12 3 12 3 12 3 4 12 4 12 4 12 4 12 4 12 4 12 12 3 4
    Down: 0 13 3 12 4 12 4 12 4 4 12 4 12 4 12 4 12 4 12 12 4 4 12 4
*/

Coroutine read(Loop &loop, Buffer &buffer) {
    while (buffer.ready()) {
        co_await buffer.read();

        // debug output raw data
        for (uint8_t time : buffer) {
            debug::out << dec(time) << ',';
        }
        debug::out << '\n';

        {
            nec::Packet packet;
            if (nec::decode(buffer.array<const uint8_t>(), packet)) {
                debug::out << "NEC " << hex(packet.address1) << ' ' << hex(packet.address2) << ' ' << hex(packet.command) << '\n';
            }
        }

        {
            uint16_t packet;
            if (nubert::decode(buffer.array<const uint8_t>(), packet)) {
                debug::out << "Nubert " << hex(packet) << '\n';
            }
        }

        {
            rc6::Packet packet;
            if (rc6::decode(buffer.array<const uint8_t>(), packet)) {
                debug::out << "RC6 " << dec(packet.mode) << ' ' << dec(packet.trailer) << ' ' << hex(packet.control) << ' ' << hex(packet.data) << '\n';
            }
        }
    }
}


int main() {
    debug::out << "IrReceiverTest\n";

    // start two coroutines so that one is receiving while the other is busy decoding a packet
    read(drivers.loop, drivers.buffer1);
    read(drivers.loop, drivers.buffer2);

    drivers.loop.run();
    return 0;
}
