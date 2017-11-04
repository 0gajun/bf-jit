#ifndef JIT_UTILS_H
#define JIT_UTILS_H

#include <vector>
#include <cstdint>
#include <memory>

class JitProgram {
public:
    JitProgram(const std::vector<uint8_t>& code);
    ~JitProgram();

    void* program_memory() {
        return program_memory_;
    }

    size_t program_size() {
        return program_size_;
    }

private:
    void* program_memory_ = nullptr;
    size_t program_size_ = 0;
};

class CodeEmitter {
public:
    CodeEmitter() = default;

    void EmitByte(uint8_t v);
    void EmitBytes(std::initializer_list<uint8_t> bytes);
    void EmitUint32(uint32_t v);
    void EmitUint64(uint64_t v);
    void ReplaceByteAtOffset(size_t offset, uint8_t v);
    void ReplaceUint32AtOffset(size_t offset, uint32_t v);

    size_t size() const {
        return code_.size();
    }

    const std::vector<uint8_t> code() {
        return code_;
    }

private:
    std::vector<uint8_t> code_;
};

uint32_t compute_relative_32bit_offset(size_t jump_from, size_t jump_to);
#endif
