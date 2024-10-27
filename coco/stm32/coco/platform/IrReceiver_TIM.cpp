#include "IrReceiver_TIM.hpp"
#include <coco/debug.hpp>
#include <algorithm>


namespace coco {

// IrReceiver_TIM

IrReceiver_TIM::IrReceiver_TIM(Loop_Queue &loop, gpio::Config inputPin, const TimerInfo &timerInfo, int timerChannel,
    Hertz<> timerClock)
    : BufferDevice(State::READY)
    , loop_(loop)
    , timerIrq_(timerInfo.irq<timer::Irq::CC>())
{
    // configure timer input pin
    gpio::enableAlternate(inputPin);

    // configure timer
    timer_ = timerInfo.enableClock()
        .setCountDuration(timerClock, 50us)
        //.setTriggerMode(timer::TriggerMode::RESET_START, timerChannel == 1 ? timer::Trigger::INPUT1 : timer::Trigger::INPUT2)
        .setTriggerMode(timer::TriggerMode::RESET, timerChannel == 1 ? timer::Trigger::INPUT1 : timer::Trigger::INPUT2)
        .enableInput1(timer::InputConfig::EDGE_BOTH | timer::InputConfig::CAPTURE, (timerChannel == 1 ? timer::InputMode::DEFAULT : timer::InputMode::ALTERNATE))
        .enableInput2(timer::InputConfig::EDGE_BOTH)
        .setCompare3(240) // 12ms timeout
        .set(timer::Interrupt::CAPTURE1 | timer::Interrupt::COMPARE3, timer::DmaRequest::NONE);
    nvic::setPriority(timerIrq_, nvic::Priority::MEDIUM); // interrupt gets enabled in first call to start()
}

IrReceiver_TIM::~IrReceiver_TIM() {
}

int IrReceiver_TIM::getBufferCount() {
    return buffers_.count();
}

IrReceiver_TIM::BufferBase &IrReceiver_TIM::getBuffer(int index) {
    return buffers_.get(index);
}

void IrReceiver_TIM::TIM_IRQHandler() {
    auto timer = timer_;
    auto status = timer.status();
    timer.clear(timer::Status::CAPTURE1 | timer::Status::COMPARE3);
    if (data_ != nullptr && (status & timer::Status::CAPTURE1) != 0) {
        // detected an edge on the data pin
        int value = timer->CCR1;

        // start timer (necessary when timer::TriggerMode::RESET_START is not supported)
        timer.start();

        // store into buffer
        *data_ = value;
        ++data_;
        if (--count_ <= 0) {
            // end of buffer
            transfers_.pop(
                [this](BufferBase &buffer) {
                    // buffer size is capacity
                    buffer.size_ = buffer.capacity_;

                    // pass buffer to event loop so that the application can be notified
                    loop_.push(buffer);
                    return true;
                },
                [](BufferBase &next) {
                    // start next buffer
                    next.start();
                }
            );
        }

        debug::setGreen();
    }
    if ((status & timer::Status::COMPARE3) != 0) {
        // timeout
        timer.stop().update();

        transfers_.pop(
            [this](BufferBase &buffer) {
                if (data_ == nullptr) {
                    // buffer was added while timer was running (therefore we missed a packet): start
                    buffer.start();
                    return false;
                } else {
                    // end of transfer: indicate that no buffer is active
                    data_ = nullptr;

                    // buffer size is number of received bytes
                    buffer.size_ = buffer.capacity_ - count_;

                    // pass buffer to event loop so that the application can be notified
                    loop_.push(buffer);
                    return true;
                }
            },
            [](BufferBase &next) {
                // start next buffer
                next.start();
            }
        );

        debug::clearGreen();
    }
}


// IrReceiver_TIM::BufferBase

IrReceiver_TIM::BufferBase::BufferBase(uint8_t *data, int capacity, IrReceiver_TIM &device)
    : coco::Buffer(data, capacity, BufferBase::State::READY), device_(device)
{
    device.buffers_.add(*this);
}

IrReceiver_TIM::BufferBase::~BufferBase() {
}

bool IrReceiver_TIM::BufferBase::start(Op op) {
    if (st.state != State::READY || (op & Op::READ) == 0 || size_ == 0) {
        assert(st.state != State::BUSY);
        return false;
    }

    // check if READ or WRITE flag is set
    assert((op & Op::READ_WRITE) != 0);

    op_ = op;
    auto &device = device_;

    {
        nvic::Guard guard(device.timerIrq_);

        // add to list of pending transfers and start immediately if list was empty
        if (device.transfers_.push(*this)) {
            if (!device.timer_.running())
                start();
        }
    }

    // set state
    setBusy();

    return true;
}

bool IrReceiver_TIM::BufferBase::cancel() {
    if (st.state != State::BUSY)
        return false;
    auto &device = device_;

    // remove from pending transfers if not yet started, otherwise complete normally
    if (device.transfers_.remove(nvic::Guard(device.timerIrq_), *this, false))
        setReady(0);

    return true;
}

void IrReceiver_TIM::BufferBase::start() {
    auto &device = device_;

    device.data_ = data_;
    device.count_ = size_;
}

void IrReceiver_TIM::BufferBase::handle() {
    setReady();
}

} // namespace coco
