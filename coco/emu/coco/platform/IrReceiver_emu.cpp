#include "IrReceiver_emu.hpp"
#include <coco/platform/GuiDpad.hpp>
#include <coco/StringBuffer.hpp>
#include <coco/StreamOperators.hpp>
#include <iostream>


namespace coco {

IrReceiver_emu::IrReceiver_emu(Loop_emu &loop, const Config &config, int id)
    : BufferDevice(State::READY)
    , loop_(loop)
    , config_(config)
    , id_(id)
{
    loop.guiHandlers.add(*this);
}

IrReceiver_emu::~IrReceiver_emu() {
}

int IrReceiver_emu::getBufferCount() {
    return buffers_.count();
}

IrReceiver_emu::Buffer &IrReceiver_emu::getBuffer(int index) {
    return buffers_.get(index);
}

// gets called regularly from the event loop
void IrReceiver_emu::handle(Gui &gui) {
    // add D-Pad to GUI
    auto result = gui.widget<GuiDpad>(id_,
        true); // center button

    for (int i = 0; i < 5; ++i) {
        // check if the button changed and is true
        if (result.buttons[i] && *result.buttons[i]) {
            auto &message = config_.messages[i];
            transfers_.pop([&message](auto &buffer) {
                auto data = buffer.data();
                int count = std::min(message.size(), buffer.capacity());
                std::ranges::copy_n(message.begin(), count, data);
                buffer.setSuccess(count);
                buffer.setReady();
            });
        }
    }
}


// Buffer

IrReceiver_emu::Buffer::Buffer(int capacity, IrReceiver_emu &device)
    : coco::Buffer(new uint8_t[capacity], capacity, State::READY)
    , device_(device)
{
    device.buffers_.add(*this);
}

IrReceiver_emu::Buffer::~Buffer() {
    delete [] data_;
}

bool IrReceiver_emu::Buffer::start() {
    if (state_ != State::READY || (op_ & Op::READ) == 0 || size_ == 0) {
        // staring a buffer that is busy is considered a bug
        assert(state_ != State::BUSY);
        setSuccess();
        return false;
    }

    // add buffer to list of transfers
    device_.transfers_.push(*this);

    // set state
    setBusy();

    return true;
}

bool IrReceiver_emu::Buffer::cancel() {
    if (state_ != State::BUSY)
        return false;

    // cancel immediately
    device_.transfers_.remove(*this);
    setError(std::errc::operation_canceled);
    setReady();

    return true;
}

} // namespace coco
