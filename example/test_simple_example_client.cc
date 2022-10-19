#include "proto/simple_example_generated.h"
#include "proto/simple_example_service.h"
#include <memory>
#include <string>

int main(int argc, char **argv) {
  std::unique_ptr<lpc::SimpleExampleClient> client_handler =
      std::make_unique<lpc::SimpleExampleClient>(64 * 1024, 64 * 1024);
  for (int i = 0; i < 100; i++) {
    lpc::MessageBuilder mb;
    std::string test_name = "test_name" + std::to_string(i);
    auto name_offset = mb.CreateString(test_name.c_str());
    auto in_vec = std::array<int, 2>{i, i + 1};
    auto in_data = lpc::SimpleExample::InputData(in_vec, 2.0f);
    auto input_offset =
        lpc::SimpleExample::CreateSimpleInput(mb, name_offset, &in_data);
    mb.Finish(input_offset);
    auto input_msg = mb.GetMessage<lpc::SimpleExample::SimpleInput>();
    lpc::Message<lpc::SimpleExample::SimpleOutput> out_msg;
    client_handler->SimpleCal(&input_msg, out_msg);
    auto out = out_msg.ToFlatbuffer();
    float expect = 2.0f * i + i + 1;
    std::cout << "out[" << i << "] output: " << out->output()
              << " while expect: " << expect << " status: " << out->status()
              << std::endl;
  }
  return 0;
}
