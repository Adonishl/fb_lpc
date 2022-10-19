#pragma once
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <flatbuffers/base.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <flatbuffers/flatbuffers.h>
namespace lpc {

// flatbuffer in C++ only support root_type for GetRoot/Unpack, that is very
// inconvinient for us to do the serialize/deserialize work. Thus we import
// Message&MessageBuilder to make a flatbuffer data easy to copy. Much of the
// implementation are borrow from flatbuffer-grpc, but much simplified and
// independent to grpc
template <class T> class Message {
public:
  Message() {}
  Message(uint8_t *ptr, flatbuffers::uoffset_t size) : ptr_(ptr), size_(size) {}
  Message(Message &&other) = default;
  Message(const Message &other) = delete;
  Message &operator=(Message &&other) = default;

  const T *ToFlatbuffer() const { return flatbuffers::GetRoot<T>(ptr_); }
  const size_t size() const { return size_; }
  uint8_t *mutable_data() { return ptr_; }
  const uint8_t *data() const { return ptr_; }

private:
  uint8_t *ptr_;
  flatbuffers::uoffset_t size_;
};

class MessageBuilder : public flatbuffers::FlatBufferBuilder {
public:
  explicit MessageBuilder(size_t initial_size = 1024)
      : flatbuffers::FlatBufferBuilder(initial_size) {}
  MessageBuilder(const MessageBuilder &other) = delete;
  MessageBuilder &operator=(const MessageBuilder &other) = delete;
  MessageBuilder(MessageBuilder &&other) : FlatBufferBuilder() { Swap(other); }
  ~MessageBuilder() = default;

  template <class T> Message<T> GetMessage() {
    auto buf_data = buf_.scratch_data();
    auto buf_size = buf_.capacity();
    auto msg_data = buf_.data();
    auto msg_size = buf_.size();
    FLATBUFFERS_ASSERT(msg_data);
    FLATBUFFERS_ASSERT(msg_size);
    FLATBUFFERS_ASSERT(msg_data >= buf_data);
    FLATBUFFERS_ASSERT(msg_data + msg_size <= buf_data + buf_size);
    Message<T> msg(msg_data, msg_size);
    return msg;
  }

  template <class T> Message<T> ReleaseMessage() {
    Message<T> msg = GetMessage<T>();
    Reset();
    return msg;
  }
};

} // namespace lpc