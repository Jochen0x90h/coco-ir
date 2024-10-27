#include <coco/Array.hpp>
#include <coco/BufferDevice.hpp>
#include <coco/IntrusiveQueue.hpp>
#include <coco/platform/Loop_emu.hpp>
#include <string>


namespace coco {

/// @brief Implementation of an emulated IR receiver.
/// Shows some buttons on the emulator gui that emit messages when pressed.
/// Currently the buttons are fixed (cross with central button)
class IrReceiver_emu : public BufferDevice, public Loop_emu::GuiHandler {
public:
    /// Messages to emit when buttons are pressed
    struct Config {
        Array<const uint8_t> messages[5];
    };

    /// @brief Constructor
    /// @param loop event loop
    /// @param messages messages to emit when buttons are pressed
    /// @param id unique id for gui
    IrReceiver_emu(Loop_emu &loop, const Config &config, int id);

    ~IrReceiver_emu() override;


    /// @brief Buffer for transferring data to/from emulated iR receiver device
    ///
    class Buffer : public coco::Buffer, public IntrusiveListNode, public IntrusiveQueueNode {
        friend class IrReceiver_emu;
    public:
        /// @brief Constructor
        /// @param headerCapacity capacity of the header
        /// @param capacity capacity of the buffer
        /// @param channel channel to attach to
        Buffer(int capacity, IrReceiver_emu &device);
        ~Buffer() override;

        bool start(Op op) override;
        bool cancel() override;

    protected:

        IrReceiver_emu &device_;
        Op op_;
    };


    // BufferDevice methods
    int getBufferCount() override;
    Buffer &getBuffer(int index) override;

protected:
    void handle(Gui &gui) override;

    Loop_native &loop_;

    Config config_;
    int id_;
    int sequenceNumber_ = 0;

    // list of buffers
    IntrusiveList<Buffer> buffers_;

    // list of active transfers
    IntrusiveQueue<Buffer> transfers_;
};

} // namespace coco
