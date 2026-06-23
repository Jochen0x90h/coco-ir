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
            // buffer full
            auto b = transfers_.pop(
                [](BufferBase &next) {
                    // start next buffer
                    next.startRx();
                });
            if (b != nullptr) {
                auto &buffer = *b;
                buffer.setSuccess(buffer.capacity_);

                // pass buffer to event loop so that the application can be notified
                loop_.push(buffer);
            }
        }

        debug::setGreen();
    }
    if ((status & timer::Status::COMPARE3) != 0) {
        // timeout
        timer.stop().update();

        auto buffer = transfers_.popIf(
            [this](BufferBase &buffer) {
                if (data_ == nullptr) {
                    // buffer was added while timer was running (which means we missed a packet): start
                    buffer.startRx();
                    return false;
                } else {
                    // end of transfer: indicate that no buffer is active
                    data_ = nullptr;

                    // buffer size is number of received bytes
                    buffer.setSuccess(buffer.capacity_ - count_);
                    return true;
                }
            },
            [](BufferBase &next) {
                // start next buffer
                next.startRx();
            });
        if (buffer != nullptr) {
            // pass buffer to event loop so that the application can be notified
            loop_.push(*buffer);
        }

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

bool IrReceiver_TIM::BufferBase::start() {
    if (state_ != State::READY) {
        assert(false);
        setError(std::errc::resource_unavailable_try_again);
        return false;
    }
    if ((op_ & Op::READ) == 0 || size_ == 0) {
        setSuccess();
        return false;
    }

    auto &device = device_;

    {
        nvic::Guard guard(device.timerIrq_);

        // add to list of pending transfers and start immediately if list was empty
        if (device.transfers_.push(*this)) {
            if (!device.timer_.running())
                startRx();
        }
    }

    // set state
    setBusy();

    return true;
}

bool IrReceiver_TIM::BufferBase::cancel() {
    if (state_ != State::BUSY)
        return false;
    auto &device = device_;

    // remove from pending transfers if not yet started, otherwise complete normally
    if (device.transfers_.guardedRemoveExceptFirst(nvic::Guard(device.timerIrq_), *this)) {
        setError(std::errc::operation_canceled);
        setReady();
    }

    return true;
}

void IrReceiver_TIM::BufferBase::onCompletion() {
    setReady();
}

} // namespace coco
