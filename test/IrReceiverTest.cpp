#include <coco/debug.hpp>
#include <coco/StreamOperators.hpp>
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

        //co_await loop.sleep(1s);
        //debug::toggleGreen();

        for (uint8_t time : buffer) {
            debug::out << dec(time) << ",";
        }
        debug::out << '\n';
    }
}


int main() {
    read(drivers.loop, drivers.buffer1);
    read(drivers.loop, drivers.buffer2);

    drivers.loop.run();
    return 0;
}
