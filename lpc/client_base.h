#pragma once
#include "message.h"
#include "shared_memory.h"
#include <memory>
#include <string>
namespace lpc {
class ClientBase {
public:
  ClientBase(unsigned int input_buffer_size, unsigned int output_buffer_size)
      : input_buffer_size_(input_buffer_size),
        output_buffer_size_(output_buffer_size) {
    // input_mem_ is used to send input data
    input_mem_ = std::make_unique<SharedMemory>("/tmp/lpc-input", false,
                                                input_buffer_size_);
    // output_mem_ is used to get output data
    output_mem_ = std::make_unique<SharedMemory>("/tmp/lpc-output", false,
                                                 output_buffer_size_);
  }

protected:
  template <class T, class P>
  void Call(unsigned int func_id, const Message<T> *in_msg,
            Message<P> &out_msg) {
    // send msg by shared memory
    input_mem_->WriteFunc(in_msg->data(), in_msg->size(), func_id);
    // (NOTE huangliang) class message right now dont hold the mem, we should
    // allocate buffer before message ctor
    unsigned int output_size = 0;
    char *output = output_mem_->Read(output_size);
    out_msg = Message<P>((uint8_t *)output, output_size);
  }
  std::unique_ptr<SharedMemory> input_mem_;
  std::unique_ptr<SharedMemory> output_mem_;
  unsigned int input_buffer_size_;
  unsigned int output_buffer_size_;
};
} // namespace lpc