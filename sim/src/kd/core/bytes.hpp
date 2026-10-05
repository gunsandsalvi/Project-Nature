// Bytes for files (A3.7): fields written one by one, little-endian, never as raw memory, so the same state makes the
// same bytes on every build; and read back with every read checked, so a short or damaged file is refused rather than
// misread.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace kd {

/// Implements RES-05, see A3.7: writes fields little-endian.
class ByteWriter {
public:
    void u8(std::uint8_t v) { out_.push_back(static_cast<std::byte>(v)); }
    void u32(std::uint32_t v) {
        for (unsigned i = 0; i < 4; ++i) {
            u8(static_cast<std::uint8_t>(v >> (8U * i)));
        }
    }
    void u64(std::uint64_t v) {
        for (unsigned i = 0; i < 8; ++i) {
            u8(static_cast<std::uint8_t>(v >> (8U * i)));
        }
    }
    void i64(std::int64_t v) { u64(static_cast<std::uint64_t>(v)); }

    [[nodiscard]] const std::vector<std::byte>& bytes() const { return out_; }

private:
    std::vector<std::byte> out_;
};

/// Implements RES-05, see A3.7: reads fields little-endian; a read past the end fails and leaves the reader failed.
class ByteReader {
public:
    explicit ByteReader(std::span<const std::byte> in) : in_(in) {}

    bool u8(std::uint8_t& v) {
        if (failed_ || at_ >= in_.size()) {
            failed_ = true;
            return false;
        }
        v = static_cast<std::uint8_t>(in_[at_++]);
        return true;
    }
    bool u32(std::uint32_t& v) {
        v = 0;
        for (unsigned i = 0; i < 4; ++i) {
            std::uint8_t b = 0;
            if (!u8(b)) {
                return false;
            }
            v |= static_cast<std::uint32_t>(b) << (8U * i);
        }
        return true;
    }
    bool u64(std::uint64_t& v) {
        v = 0;
        for (unsigned i = 0; i < 8; ++i) {
            std::uint8_t b = 0;
            if (!u8(b)) {
                return false;
            }
            v |= static_cast<std::uint64_t>(b) << (8U * i);
        }
        return true;
    }
    bool i64(std::int64_t& v) {
        std::uint64_t u = 0;
        const bool ok = u64(u);
        v = static_cast<std::int64_t>(u);
        return ok;
    }

    /// Whether every read so far succeeded and nothing is left unread.
    [[nodiscard]] bool finished() const { return !failed_ && at_ == in_.size(); }
    [[nodiscard]] bool failed() const { return failed_; }

private:
    std::span<const std::byte> in_;
    std::size_t at_ = 0;
    bool failed_ = false;
};

}  // namespace kd
