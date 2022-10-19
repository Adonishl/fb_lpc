
#include "simple_example_generated.h"

#include "simple_example_service.h"

namespace lpc {


void SimpleExampleServerImpl::SimpleCal(
    const lpc::Message<lpc::SimpleExample::SimpleInput> *input,
    lpc::Message<lpc::SimpleExample::SimpleOutput> &output) {
  // For user to implement
  auto input_fb = input->ToFlatbuffer();
  std::cout << "input name: " << input_fb->name()->str() << std::endl;
  auto in_v = input_fb->data()->in();
  float out = input_fb->data()->weight() * in_v->Get(0) + (float)(in_v->Get(1));

  lpc::MessageBuilder mb;
  auto output_offset = lpc::SimpleExample::CreateSimpleOutput(
      mb, lpc::SimpleExample::Status_OK, out);
  mb.Finish(output_offset);
  output = mb.GetMessage<lpc::SimpleExample::SimpleOutput>();
}


} // namespace lpc
