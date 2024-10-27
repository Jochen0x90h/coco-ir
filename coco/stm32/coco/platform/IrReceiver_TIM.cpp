#include "IrReceiver_TIM.hpp"
#include <coco/debug.hpp>
#include <algorithm>


namespace coco {

// IrReceiver_TIM

IrReceiver_TIM::IrReceiver_TIM(Loop_Queue &loop, gpio::Config inputPin, timer::Registers timer, int inputIndex, int timerIrq)
    : BufferDevice(State::READY)
    , loop(loop)
    , timerIrq(timerIrq)
{
    // configure timer input pin
    gpio::configureAlternate(inputPin);

    // configure timer
    this->timer = timer
        .setTrigger(timer::TriggerMode::RESET_START, inputIndex == 1 ? timer::Trigger::INPUT1 : timer::Trigger::INPUT2)
        .setCompare3(240) // 12ms timeout
        .setDmaInterruptEnable(timer::Enable::CAPTURE1_INTERRUPT | timer::Enable::COMPARE3_INTERRUPT)
        .setCapture1(timer::CaptureMode::EDGE_BOTH | (inputIndex == 1 ? timer::CaptureMode::INPUT_DEFAULT : timer::CaptureMode::INPUT_ALTERNATE) | timer::CaptureMode::ENABLED)
        .setCapture2(timer::CaptureMode::EDGE_BOTH | timer::CaptureMode::INPUT_DEFAULT);
    nvic::setPriority(this->timerIrq, nvic::Priority::MEDIUM); // interrupt gets enabled in first call to start()
}

IrReceiver_TIM::~IrReceiver_TIM() {
}

int IrReceiver_TIM::getBufferCount() {
    return this->buffers.count();
}

IrReceiver_TIM::BufferBase &IrReceiver_TIM::getBuffer(int index) {
    return this->buffers.get(index);
}

void IrReceiver_TIM::TIM_IRQHandler() {
    auto timer = this->timer;
    auto status = timer.getAndClearStatus(timer::Status::CAPTURE1 | timer::Status::COMPARE3);
    if (this->data != nullptr && (status & timer::Status::CAPTURE1) != 0) {
        // detected an edge on the data pin
        int value = timer->CCR1;

        *this->data = value;
        ++this->data;
        if (--this->count <= 0) {
            // end of buffer
            this->transfers.pop(
                [this](BufferBase &buffer) {
                    // buffer size is capacity
                    buffer.p.size = buffer.p.capacity;

                    // pass buffer to event loop so that the application can be notified
                    this->loop.push(buffer);
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
        timer.stop();
        timer->CNT = 0;

        this->transfers.pop(
            [this](BufferBase &buffer) {
                if (this->data == nullptr) {
                    // buffer must be added to transfers list when timer was running: start
                    buffer.start();
                    return false;
                } else {
                    // end of transfer: indicate that no buffer is active
                    this->data = nullptr;

                    // buffer size is number of received bytes
                    buffer.p.size = buffer.p.capacity - this->count;

                    // pass buffer to event loop so that the application can be notified
                    this->loop.push(buffer);
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


// BufferBase

IrReceiver_TIM::BufferBase::BufferBase(uint8_t *data, int capacity, IrReceiver_TIM &device)
    : coco::Buffer(data, capacity, BufferBase::State::READY), device(device)
{
    device.buffers.add(*this);
}

IrReceiver_TIM::BufferBase::~BufferBase() {
}

bool IrReceiver_TIM::BufferBase::start(Op op) {
    if (this->st.state != State::READY) {
        assert(this->st.state != State::BUSY);
        return false;
    }

    // check if READ or WRITE flag is set
    assert((op & Op::READ_WRITE) != 0);

    this->op = op;
    auto &device = this->device;

    {
        nvic::Guard guard(device.timerIrq);

        // add to list of pending transfers and start immediately if list was empty
        if (device.transfers.push(*this)) {
            if (!device.timer.running())
                start();
        }
    }

    // set state
    setBusy();

    return true;
}

bool IrReceiver_TIM::BufferBase::cancel() {
    if (this->st.state != State::BUSY)
        return false;
    auto &device = this->device;

    // remove from pending transfers if not yet started, otherwise complete normally
    if (device.transfers.remove(nvic::Guard(device.timerIrq), *this, false))
        setReady(0);

    return true;
}

void IrReceiver_TIM::BufferBase::start() {
    auto &device = this->device;

    //device.timer.start();
    int headerSize = this->p.headerSize;
    device.data = this->p.data + headerSize;
    device.count = this->p.capacity - headerSize;
}

void IrReceiver_TIM::BufferBase::handle() {
    setReady();
}

} // namespace coco
