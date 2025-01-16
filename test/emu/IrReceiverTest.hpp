#pragma once

#include <coco/platform/Loop_emu.hpp>
#include <coco/platform/IrReceiver_emu.hpp>
#include <coco/board/config.hpp>



using namespace coco;

// emulated Näve lighting remote control
const uint8_t timesStar[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,32,12,10,12,32,12};
const uint8_t timesHotter[] = {0,180,89,11,33,11,11,11,11,11,11,11,11,11,11,11,33,11,11,11,11,11,33,11,11,11,33,11,11,11,33,11,33,12,33,11,33,11,11,11,11,11,11,11,33,11,11,12,10,11,11,11,11,12,33,11,33,11,33,11,11,11,33,11,33,11,33,11};
const uint8_t timesColder[] = {0,180,89,11,33,11,11,11,11,11,11,11,11,11,11,11,33,11,11,11,11,11,33,11,11,11,33,11,11,11,33,11,33,11,33,11,11,11,11,11,11,11,11,11,34,11,10,11,11,11,11,11,33,11,33,11,33,11,33,11,11,11,33,11,33,11,33,11};
const uint8_t timesPlus[] = {0,181,88,12,32,12,10,12,10,12,10,12,10,12,10,12,32,12,10,12,10,12,32,12,9,12,32,12,10,12,32,12,32,12,32,12,10,12,32,12,10,12,9,12,32,12,9,12,10,12,10,12,32,12,10,12,32,12,32,12,10,12,32,12,32,12,32,12};
const uint8_t timesMinus[] = {0,181,88,12,32,12,10,12,10,12,9,12,10,12,9,12,32,12,9,12,9,12,32,12,10,12,32,12,9,12,32,12,32,12,32,12,32,12,32,12,9,12,9,12,32,12,9,12,10,12,10,12,9,12,10,12,32,12,32,12,9,12,32,12,32,12,32,12};
IrReceiver_emu::Config config {
    {
        timesHotter, // left
        timesPlus, // up
        timesMinus, // down
        timesColder, // right
        timesStar // center
    }
};


// drivers for I2cMasterTest on STM32G431 Nucleo board
// https://www.st.com/content/ccc/resource/technical/layouts_and_diagrams/schematic_pack/group1/98/d2/70/60/b1/cb/44/4c/mb1367-g431rb-c04_schematic/files/mb1367-g431rb-c04_schematic.pdf/jcr:content/translations/en.mb1367-g431rb-c04_schematic.pdf
struct Drivers {
    Loop_emu loop;

    using IrReceiver = IrReceiver_emu;
    IrReceiver ir{loop, config, 100};
    IrReceiver::Buffer buffer1{80, ir};
    IrReceiver::Buffer buffer2{80, ir};
};

Drivers drivers;
