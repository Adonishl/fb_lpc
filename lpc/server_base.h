#pragma once
#include "shared_memory.h"
#include <cstdint>
#include <memory>
#include <string>
namespace lpc {
class ServerBase {
public:
  ServerBase(unsigned int input_buffer_size, unsigned int output_buffer_size)
      : input_buffer_size_(input_buffer_size),
        output_buffer_size_(output_buffer_size) {
    // input_mem_ is used to receive input data
    input_mem_ = std::make_unique<SharedMemory>("/tmp/lpc-input", true,
                                                input_buffer_size_);
    // output_mem_ is used to sendback output data
    output_mem_ = std::make_unique<SharedMemory>("/tmp/lpc-output", true,
                                                 output_buffer_size_);
  }

protected:
  uint8_t *GetInput(unsigned int &func_id, unsigned int &data_size) {
    return (uint8_t *)(input_mem_->ReadFunc(data_size, func_id));
  }

  void SendOutput(const void *msg, unsigned int size) {
    output_mem_->Write(msg, size);
  }
  std::unique_ptr<SharedMemory> input_mem_;
  std::unique_ptr<SharedMemory> output_mem_;
  unsigned int input_buffer_size_;
  unsigned int output_buffer_size_;
};
} // namespace lpc
