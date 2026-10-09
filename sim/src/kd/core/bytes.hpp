// Bytes for files (A3.7): fields written one by one, little-endian, never as raw memory, so the same state makes the
// same bytes on every build; and read back with every read checked, so a short or damaged file is refused rather than
// misread.
#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace kd {

/// Implements RES-05, see A3.7: writes fields little-endian.
class ByteWriter {
public:
    void u8(std::uint8_t v) { out_.push_back(static_cast<std::byte>(v)); }
    void u32(std::uint32_t v) {
        const std::size_t at = out_.size();
        out_.resize(at + 4);
        for (unsigned i = 0; i < 4; ++i) {
            out_[at + i] = static_cast<std::byte>(v >> (8U * i));
        }
    }
    void u64(std::uint64_t v) {
        const std::size_t at = out_.size();
        out_.resize(at + 8);
        for (unsigned i = 0; i < 8; ++i) {
            out_[at + i] = static_cast<std::byte>(v >> (8U * i));
        }
    }
    void i64(std::int64_t v) { u64(static_cast<std::uint64_t>(v)); }
    /// Bytes as they are, after their count.
    void blob(std::span<const std::byte> b) {
        u64(b.size());
        out_.insert(out_.end(), b.begin(), b.end());
    }
    /// A text as its bytes, after their count.
    void text(std::string_view t) {
        u64(t.size());
        for (const char c : t) {
            u8(static_cast<std::uint8_t>(c));
        }
    }

    [[nodiscard]] const std::vector<std::byte>& bytes() const { return out_; }
    /// The bytes written, leaving the writer empty.
    [[nodiscard]] std::vector<std::byte> take() { return std::exchange(out_, {}); }

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

    /// Bytes as blob() wrote them; a count beyond what is left fails.
    bool blob(std::vector<std::byte>& b) {
        std::uint64_t n = 0;
        if (!u64(n) || n > in_.size() - at_) {
            failed_ = true;
            return false;
        }
        const auto from = in_.begin() + static_cast<std::ptrdiff_t>(at_);
        b.assign(from, from + static_cast<std::ptrdiff_t>(n));
        at_ += n;
        return true;
    }
    /// A text as text() wrote it.
    bool text(std::string& t, std::size_t most = std::numeric_limits<std::size_t>::max()) {
        std::uint64_t n = 0;
        if (!u64(n) || n > most || n > in_.size() - at_) {
            failed_ = true;
            return false;
        }
        t.clear();
        for (std::uint64_t i = 0; i < n; ++i) {
            t.push_back(static_cast<char>(in_[at_++]));
        }
        return true;
    }

    /// Whether every read so far succeeded and nothing is left unread.
    [[nodiscard]] bool finished() const { return !failed_ && at_ == in_.size(); }
    [[nodiscard]] bool failed() const { return failed_; }
    [[nodiscard]] std::size_t remaining() const { return in_.size() - at_; }

private:
    std::span<const std::byte> in_;
    std::size_t at_ = 0;
    bool failed_ = false;
};

}  // namespace kd
