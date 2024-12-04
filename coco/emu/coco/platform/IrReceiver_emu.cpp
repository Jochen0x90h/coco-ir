#include "IrReceiver_emu.hpp"
#include <coco/platform/GuiDpad.hpp>
#include <coco/StringBuffer.hpp>
#include <coco/StreamOperators.hpp>
#include <iostream>


namespace coco {

IrReceiver_emu::IrReceiver_emu(Loop_emu &loop, const Config &config, int id)
    : BufferDevice(State::READY)
    , loop(loop)
    , config(config)
    , id(id)
{
    loop.guiHandlers.add(*this);
}

IrReceiver_emu::~IrReceiver_emu() {
}

int IrReceiver_emu::getBufferCount() {
	return this->buffers.count();
}

IrReceiver_emu::Buffer &IrReceiver_emu::getBuffer(int index) {
	return this->buffers.get(index);
}

// gets called regularly from the event loop
void IrReceiver_emu::handle(Gui &gui) {
	// add D-Pad to GUI
	auto result = gui.widget<GuiDpad>(this->id,
        true); // center button

    for (int i = 0; i < 5; ++i) {
        if (result.buttons[i]) {
			if (*result.buttons[i]) {
            	auto buffer = this->transfers.pop();
				if (buffer != nullptr) {
					auto data = buffer->data();
					int count = std::min(this->config.messages[i].size(), buffer->capacity());
					std::ranges::copy_n(this->config.messages[i].begin(), count, data);
					buffer->setReady(count);
				}
			}
        }
    }
}


// Buffer

IrReceiver_emu::Buffer::Buffer(int capacity, IrReceiver_emu &device)
	: coco::Buffer(new uint8_t[capacity], capacity, State::READY)
	, device(device)
{
	device.buffers.add(*this);
}

IrReceiver_emu::Buffer::~Buffer() {
	delete [] this->p.data;
}

bool IrReceiver_emu::Buffer::start(Op op) {
	if (this->st.state != State::READY) {
		// staring a buffer that is busy is considered a bug
		assert(this->st.state != State::BUSY);
		return false;
	}

	// check if READ or WRITE flag is set
	assert((op & Op::READ_WRITE) != 0);

	this->op = op;

	// add buffer to list of transfers
	this->device.transfers.push(*this);

	// set state
	setBusy();

	return true;
}

bool IrReceiver_emu::Buffer::cancel() {
	if (this->st.state != State::BUSY)
		return false;

	// cancel immediately
	this->device.transfers.remove(*this);
	setReady(0);

	return true;
}

} // namespace coco
