#include "shared_memory.h"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv) {
  auto reader_shm = std::make_unique<lpc::SharedMemory>("/tmp/shm-test", false);
  while (true) {
    unsigned int data_size = 0;
    unsigned int func_id = 0;
    auto data_ptr = reader_shm->ReadFunc(data_size, func_id);
    if (data_ptr != nullptr && data_size > 0) {
      std::cout << "data: " << std::string(data_ptr, data_size)
                << " func id: " << func_id << std::endl;
    } else {
      std::cout << "fail to read data and func" << std::endl;
    }
  }
  return 0;
}