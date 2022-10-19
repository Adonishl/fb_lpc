#pragma once
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

namespace lpc {

constexpr unsigned int size64KB = 64 * 1024;
/*
SharedMemory class here will create a handler for reader and writer
communication by shared memory method. It supplies the method like:
Read/Write, which will much ease the use.
To simplify the implementation, we use memory-mapped file's operation.
Usually mmap is associated with disk file, but you can open the file in tmpfs
like /tmp to achieve memory-level performance which is equal to shared-memory.
Unlike shmget and other API, mmap is easy to manage in docker environment
[NOTE huangliang] this method only works in linux system
*/
class SharedMemory {
public:
  // @file_name: file_name is used to connect server&client like a handler
  // @is_creator: the creator is in charge with memory space ctor/dector
  // @mem_size: is the shared memory size we allocate, usually it should be page
  // aligned for performance need. (NOTE huangliang) mem_size we assigned should
  // be a little larger than the data we transfer, cuz we use the first few
  // bytes for synchronize need and store buffer info
  // Here we use the first 8 Bytes(64bits) to store those infos, currently we
  // use the high 1 Bytes(char size) to store sync-signal and low 32bits to
  // store buffer size info. Well, the middle 31bits will be used in the future,
  // when we want to support large amount data transfer. And mem_size is smaller
  // than INT_MAX, and is 64KB by default
  SharedMemory(const std::string &file_name, bool is_creator,
               unsigned int mem_size = size64KB)
      : file_name_(file_name), is_creator_(is_creator), mem_size_(mem_size) {
    int fd_ = open(file_name_.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd_ < 0) {
      std::cerr << "Error opening file: " << file_name << std::endl;
      exit(-1);
    }
    if (is_creator_) {
      lseek(fd_, mem_size_, SEEK_SET);
      if (write(fd_, "", 1) < 1) {
        std::cerr << "Error writing to file: " << file_name << std::endl;
        exit(-1);
      }
      lseek(fd_, 0, SEEK_SET);
    }
    file_mem_ =
        mmap(NULL, mem_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if ((int64_t)file_mem_ < 0) {
      std::cerr << "Error mapping file: " << file_name << std::endl;
      exit(-1);
    }
    close(fd_);
    // we make the first Byte as sync_signal, it will used like a spin lock
    sync_ = (std::atomic_char *)file_mem_;
    // the creator will be in charge of empty the memory, also we make the
    // memory writable at the init state
    if (is_creator_) {
      memset(file_mem_, 0, mem_size_);
      sync_->store('s', std::memory_order_release);
    }
    // buffer_mem_ is 8B offset to file_mem_
    buffer_mem_ = (char *)file_mem_ + 8;
    avaiable_size_ = mem_size_ - 8;
    // we use [31:63]bit to indicate data size
    data_size_ptr_ = (unsigned int *)file_mem_ + 1;
    // we use [8:15]bit to indicate fun id if we are transfer func
    func_ptr_ = (char *)file_mem_ + 1;
  }
  ~SharedMemory() {
    munmap(file_mem_, mem_size_);
    if (is_creator_) {
      remove(file_name_.c_str());
    }
    close(fd_);
  }

  // (TODO) currently we only support the data size smaller than mem_size which
  // means only write the data once. If the data is bigger than mem_size we
  // should split it into pieces and then write them. This means the user dont
  // need to assign mem_size by himself, we might got a benchmark test to find
  // out which size whould be most efficient.
  bool Write(const std::string &data) {
    return Write(data.data(), data.size());
  }

  bool Write(const void *data_ptr, unsigned int data_size) {
    if (data_size > avaiable_size_) {
      std::cerr << "Error, currently we dont support data transfer with size: "
                << data_size << std::endl;
      return false;
    }

    // spinlock
    while (sync_->load(std::memory_order_acquire) != 's')
      ;
    memcpy(buffer_mem_, data_ptr, data_size);
    // set data size
    *data_size_ptr_ = data_size;
    // inform reader that msg is writen
    sync_->store('c', std::memory_order_release);
    return true;
  }

  // (NOTE huangliang) func_id is the function index for reader to get func
  // type, we use buffer head[8-16]bits to store the info, this means func_id
  // must be smaller than 256
  bool WriteFunc(const void *data_ptr, unsigned int data_size,
                 unsigned int func_id) {
    if (data_size > avaiable_size_) {
      std::cerr << "Error, currently we dont support data transfer with size: "
                << data_size << std::endl;
      return false;
    }

    // spinlock
    while (sync_->load(std::memory_order_acquire) != 's')
      ;
    memcpy(buffer_mem_, data_ptr, data_size);
    // set data size
    *data_size_ptr_ = data_size;
    // set func id
    *func_ptr_ = (char)func_id;
    // inform reader that msg is writen
    sync_->store('c', std::memory_order_release);
    return true;
  }

  // Unlike Write functions which will copy data to shared memory, Read
  // functions will not copy the data, they only supply the pointer and size of
  // data on the shared memory
  char *Read(unsigned int &data_size) {
    while (sync_->load(std::memory_order_acquire) != 'c')
      ;
    // get data size
    data_size = *data_size_ptr_;
    // inform client that msg is read
    sync_->store('s', std::memory_order_release);
    return buffer_mem_;
  }

  char *ReadFunc(unsigned int &data_size, unsigned int &func_id) {
    while (sync_->load(std::memory_order_acquire) != 'c')
      ;
    // get data size
    data_size = *data_size_ptr_;
    // get func id
    func_id = (unsigned int)(*func_ptr_);
    // inform client that msg is read
    sync_->store('s', std::memory_order_release);
    return buffer_mem_;
  }

private:
  std::string file_name_;
  bool is_creator_;
  unsigned int mem_size_;
  int fd_;
  void *file_mem_;
  char *buffer_mem_;
  char *func_ptr_;
  unsigned int *data_size_ptr_;
  int avaiable_size_;
  std::atomic_char *sync_;
};
} // namespace lpc