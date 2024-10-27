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
        if (result.buttons[i]) {
            if (*result.buttons[i]) {
                auto buffer = transfers_.pop();
                if (buffer != nullptr) {
                    auto data = buffer->data();
                    int count = std::min(config_.messages[i].size(), buffer->capacity());
                    std::ranges::copy_n(config_.messages[i].begin(), count, data);
                    buffer->setReady(count);
                }
            }
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

bool IrReceiver_emu::Buffer::start(Op op) {
    if (st.state != State::READY) {
        // staring a buffer that is busy is considered a bug
        assert(st.state != State::BUSY);
        return false;
    }

    // check if READ or WRITE flag is set
    assert((op & Op::READ_WRITE) != 0);

    op_ = op;

    // add buffer to list of transfers
    device_.transfers_.push(*this);

    // set state
    setBusy();

    return true;
}

bool IrReceiver_emu::Buffer::cancel() {
    if (st.state != State::BUSY)
        return false;

    // cancel immediately
    device_.transfers_.remove(*this);
    setReady(0);

    return true;
}

} // namespace coco
