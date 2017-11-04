#include "jit_utils.h"

#include <iostream>
#include <cassert>
#include <cstring>
#include <sys/mman.h>

void* alloc_writable_memory(size_t size) {
    void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return nullptr;
    }
    return ptr;
}

int make_memory_executable(void* m, size_t size) {
    if (mprotect(m, size, PROT_READ | PROT_EXEC) < 0) {
        perror("mprotect");
        return -1;
    }
    return 0;
}

JitProgram::JitProgram(const std::vector<uint8_t>& code) {
    program_size_ = code.size();
    program_memory_ = alloc_writable_memory(program_size_);
    if (program_memory_ == nullptr) {
        std::cerr << "Unable to allocate writable memory";
        exit(1);
    }
    memcpy(program_memory_, code.data(), program_size_);
    if (make_memory_executable(program_memory_, program_size_) < 0) {
        std::cerr << "Unable to mark memory as executable";
        exit(1);
    }
}

JitProgram::~JitProgram() {
    if (program_memory_ != nullptr) {
        if (munmap(program_memory_, program_size_) < 0) {
            perror("munmap");
            std::cerr << "Unable to munmap program memory";
            exit(1);
        }
    }
}

void CodeEmitter::EmitByte(uint8_t v) {
    code_.push_back(v);
}


void CodeEmitter::EmitBytes(std::initializer_list<uint8_t> bytes) {
    for (auto b : bytes) {
        EmitByte(b);
    }
}

void CodeEmitter::EmitUint32(uint32_t v) {
    EmitByte(v & 0xFF);
    EmitByte((v >> 8) & 0xFF);
    EmitByte((v >> 16) & 0xFF);
    EmitByte((v >> 24) & 0xFF);
}

void CodeEmitter::EmitUint64(uint64_t v) {
    EmitUint32(v & 0xFFFFFFFF);
    EmitUint32((v >> 32) & 0xFFFFFFFF);
}

void CodeEmitter::ReplaceByteAtOffset(size_t offset, uint8_t v) {
  assert(offset < code_.size() && "replacement fits in code");
  code_[offset] = v;
}

void CodeEmitter::ReplaceUint32AtOffset(size_t offset, uint32_t v) {
  ReplaceByteAtOffset(offset, v & 0xFF);
  ReplaceByteAtOffset(offset + 1, (v >> 8) & 0xFF);
  ReplaceByteAtOffset(offset + 2, (v >> 16) & 0xFF);
  ReplaceByteAtOffset(offset + 3, (v >> 24) & 0xFF);
}

uint32_t compute_relative_32bit_offset(size_t jump_from, size_t jump_to) {
    if (jump_to >= jump_from) {
        size_t diff = jump_to - jump_from;
        assert(diff < (1ull << 31));
        return diff;
    } else {
        size_t diff = jump_from - jump_to;
        assert(diff - 1 < (1ull << 31));
        uint32_t diff_unsigned = static_cast<uint32_t>(diff);
        return ~diff_unsigned + 1;
    }
}
