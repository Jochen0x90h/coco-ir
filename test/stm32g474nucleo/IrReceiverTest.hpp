#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/IrReceiver_TIM.hpp>
#include <coco/board/config.hpp>



using namespace coco;


// drivers for I2cMasterTest on STM32G431 Nucleo board
// https://www.st.com/content/ccc/resource/technical/layouts_and_diagrams/schematic_pack/group1/98/d2/70/60/b1/cb/44/4c/mb1367-g431rb-c04_schematic/files/mb1367-g431rb-c04_schematic.pdf/jcr:content/translations/en.mb1367-g431rb-c04_schematic.pdf
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using IrReceiver = IrReceiver_TIM;
    IrReceiver ir{loop,
        // TIM1
        //gpio::Config::PC0 | gpio::Config::AF2, // data pin TIM1_CH1 (CN8 6) (don't forget to lookup the alternate function number in the data sheet!)
        //timer::TIM1_INFO, // (don't forget to implement TIM1_CC_IRQHandler())
        //1, // input 1

        // TIM3
        gpio::Config::PC6 | gpio::Config::AF2, // data pin TIM3_CH1 (CN10 4)
        timer::TIM3_INFO, // (don't forget to implement TIM3_IRQHandler())
        1, // input 1

        // TIM5
        //gpio::Config::PA0 | gpio::Config::AF2, // data pin TIM5_CH1 (CN8 1)
        //gpio::Config::PA1 | gpio::Config::AF2, // data pin TIM5_CH2 (CN8 2)
        //timer::TIM5_INFO, // (don't forget to implement TIM5_IRQHandler())
        //1, // input 1
        //2, // input 2

        APB1_TIMER_CLOCK};
    IrReceiver::Buffer<80> buffer1{ir};
    IrReceiver::Buffer<80> buffer2{ir};
};

Drivers drivers;

extern "C" {
//void TIM1_CC_IRQHandler() {
//    drivers.ir.TIM_IRQHandler();
//}
void TIM3_IRQHandler() {
    drivers.ir.TIM_IRQHandler();
}
//void TIM5_IRQHandler() {
//    drivers.ir.TIM_IRQHandler();
//}
}
