#include "proto/simple_example_service.h"
#include <memory>

int main(int argc, char **argv) {
  std::unique_ptr<lpc::SimpleExampleServer> server_handler =
      std::make_unique<lpc::SimpleExampleServer>(64 * 1024, 64 * 1024);
  server_handler->Serve();
  return 0;
}
