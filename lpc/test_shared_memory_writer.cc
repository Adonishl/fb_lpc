#include "shared_memory.h"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv) {
  auto writer_shm = std::make_unique<lpc::SharedMemory>("/tmp/shm-test", true);
  std::string write_data_base = "hello";
  for (int i = 0; i < 100; i++) {
    std::string to_write = write_data_base + std::to_string(i);
    if (!writer_shm->WriteFunc(to_write.data(), to_write.size(),
                               (unsigned int)i)) {
      std::cout << "Error writing: " << to_write << " func id: " << i
                << std::endl;
    }
  }
  return 0;
}