#include "../example/proto/simple_example_generated.h"
#include "message.h"
#include <iostream>

int main(int argc, char **argv) {
  // Create a SimpleInput in MessageBuilder
  lpc::MessageBuilder mb;
  auto name_offset = mb.CreateString("test_name");
  auto in_vec = std::array<int, 2>{1, 2};
  auto in_data = lpc::SimpleExample::InputData(in_vec, 2.0f);
  auto input_offset =
      lpc::SimpleExample::CreateSimpleInput(mb, name_offset, &in_data);
  mb.Finish(input_offset);
  auto input_msg = mb.GetMessage<lpc::SimpleExample::SimpleInput>();
  // copy to mem
  uint8_t mem[1024];
  std::cout << "Copy to mem with data size: " << input_msg.size() << std::endl;
  memcpy(&mem[0], input_msg.data(), input_msg.size());
  
  // regenerate
  auto msg = lpc::Message<lpc::SimpleExample::SimpleInput>(&mem[0], input_msg.size());
  auto fb = msg.ToFlatbuffer();
  std::cout << "fb name: " << fb->name()->str() << std::endl;
  std::cout << "fb weight: " << fb->data()->weight() << std::endl;
  auto in_v = fb->data()->in();
  std::cout << "fb in[0]: " << in_v->Get(0) << std::endl;
  std::cout << "fb in[1]: " << in_v->Get(1) << std::endl;
  return 0;
}