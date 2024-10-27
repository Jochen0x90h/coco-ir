#pragma once

#include <coco/BufferDevice.hpp>
#include <coco/align.hpp>
#include <coco/InterruptQueue.hpp>
#include <coco/platform/Loop_Queue.hpp>
#include <coco/platform/dma.hpp>
#include <coco/platform/gpio.hpp>
#include <coco/platform/timer.hpp>
#include <coco/platform/nvic.hpp>


namespace coco {

/// @param Implementation of I2C hardware interface for stm32f0 and stm32g4 with multiple virtual channels.
///
/// Reference manual:
///   f0: https://www.st.com/resource/en/reference_manual/dm00031936-stm32f0x1stm32f0x2stm32f0x8-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
///   g4: https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
/// Data sheet:
///   f042: https://www.st.com/resource/en/datasheet/stm32f042f6.pdf
///   f051: https://www.st.com/resource/en/datasheet/dm00039193.pdf
///   g431: https://www.st.com/resource/en/datasheet/stm32g431rb.pdf
///   g474: https://www.st.com/resource/en/datasheet/stm32g474cb.pdf
/// Resources:
///   TIM
class IrReceiver_TIM : public BufferDevice {
protected:
    // require timer with 4 capture/compare channels and capture/compare interrupt
    using TimerInfo = timer::Info<timer::Feature::CC1_4, timer::Irq::CC>;

public:
    /// @brief Constructor
    /// @param loop event loop
    /// @param inputPin input pin from the IR receiver, must be channel 1 or 2 of a timer
    /// @param timerInfo info of timer instance to use
    /// @param timerChannel channel index of the timer, 1 or 2
    /// @param timerClock timer clock (e.g. APB1_TIMER_CLOCK or APB2_TIMER_CLOCK, depending on whether the timer is clocked by APB1 or APB2)
    IrReceiver_TIM(Loop_Queue &loop, gpio::Config inputPin, const TimerInfo &timerInfo, int timerChannel, Hertz<> timerClock);

    /// @brief Destructor
    ///
    ~IrReceiver_TIM() override;


    // internal buffer base class, derives from IntrusiveListNode for the list of buffers and Loop_Queue::Handler to be notified from the event loop
    class BufferBase : public coco::Buffer, public IntrusiveListNode, public Loop_Queue::Handler {
        friend class IrReceiver_TIM;
    public:
        /// @brief Constructor
        /// @param data data of the buffer
        /// @param capacity capacity of the buffer
        /// @param device device to attach to
        BufferBase(uint8_t *data, int capacity, IrReceiver_TIM &device);
        ~BufferBase() override;

        // Buffer methods
        bool start(Op op) override;
        bool cancel() override;

    protected:
        void start();
        void handle() override;

        IrReceiver_TIM &device_;
        Op op_;
    };

    /// @brief Buffer for transferring data to/from a I2C slave.
    /// @tparam C capacity of buffer
    template <int C>
    class Buffer : public BufferBase {
    public:
        Buffer(IrReceiver_TIM &device) : BufferBase(data, C, device) {}

    protected:
        alignas(4) uint8_t data[C];
    };


    // BufferDevice methods
    int getBufferCount() override;
    BufferBase &getBuffer(int index) override;

    /// @brief handle timer interrupt, needs to be called from timer capture/compare interrupt handler
    /// (e.g. TIM1_CC_IRQHandler() or TIM3_IRQHandler())
    void TIM_IRQHandler();
protected:

    Loop_Queue &loop_;

    // i2c
    TimerInfo::Instance timer_;
    int timerIrq_;

    // list of buffers
    IntrusiveList<BufferBase> buffers_;

    // list of active transfers
    InterruptQueue<BufferBase> transfers_;

    uint8_t *data_ = nullptr;
    int count_;
};

} // namespace coco
