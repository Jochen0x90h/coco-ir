#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/IrReceiver_TIM.hpp>
#include <coco/board/config.hpp>



using namespace coco;


/// @brief Drivers for IrReceiverTest on STM32G431 Nucleo board.
/// https://www.st.com/en/evaluation-tools/nucleo-g431rb.html
///
/// Connect IR receiver module like this:
/// GND -> CN6 6 or CN10 20
/// 3V3 -> CN6 4
/// Data -> (depending on used timer, see below)
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using IrReceiver = IrReceiver_TIM;
    IrReceiver ir{loop,
        // select timer, also adjust timer clock (e.g. APB1_TIMER_CLOCK) and IRQ handler (e.g. TIM3_IRQHandler())

        // TIM1
        //gpio::PC0 | gpio::AF2, // data pin TIM1_CH1 (CN8 6) (don't forget to lookup the alternate function number in the data sheet!)
        //timer::TIM1_INFO,
        //1, // channel 1

        // TIM3
        gpio::PC6 | gpio::AF2, // data pin TIM3_CH1 (CN10 4)
        timer::TIM3_INFO,
        1, // channel 1

        // TIM5
        //gpio::PA0 | gpio::AF2, // data pin TIM5_CH1 (CN8 1)
        //gpio::PA1 | gpio::AF2, // data pin TIM5_CH2 (CN8 2)
        //timer::TIM5_INFO,
        //1, // channel 1
        //2, // channel 2

        APB1_TIMER_CLOCK}; // TIM3, TIM5
        //APB2_TIMER_CLOCK}; // TIM1
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
