#include "kd/save/log.hpp"

#include "kd/core/bytes.hpp"
#include "kd/core/check.hpp"
#include "kd/num/digest.hpp"

namespace kd::save {

namespace {

std::uint64_t checksum(std::uint32_t length, std::uint32_t type, std::uint64_t sequence,
                       std::span<const std::byte> body) {
    num::Digest d;
    d.u32(length);
    d.u32(type);
    d.u64(sequence);
    d.bytes(body);
    return d.value();
}

}  // namespace

Bytes frame(std::uint32_t type, std::uint64_t sequence, std::span<const std::byte> body) {
    KD_CHECK(body.size() < (std::uint64_t{1} << 31U), "save::frame: a record's body is under 2 GiB");
    const auto length = static_cast<std::uint32_t>(body.size());
    ByteWriter w;
    w.u32(length);
    w.u32(type);
    w.u64(sequence);
    w.u64(checksum(length, type, sequence, body));
    Bytes out = w.take();
    out.insert(out.end(), body.begin(), body.end());
    return out;
}

LogRead read_log(std::span<const std::byte> bytes, std::uint64_t first, bool allow_gaps) {
    LogRead out;
    std::uint64_t at = 0;
    std::uint64_t next = first;
    while (at < bytes.size()) {
        if (bytes.size() - at < kFrame) {
            out.cut = true;
            break;
        }
        ByteReader r(bytes.subspan(at, kFrame));
        std::uint32_t length = 0;
        std::uint32_t type = 0;
        std::uint64_t sequence = 0;
        std::uint64_t sum = 0;
        r.u32(length);
        r.u32(type);
        r.u64(sequence);
        r.u64(sum);
        if (length > bytes.size() - at - kFrame) {
            out.cut = true;
            break;
        }
        const std::span<const std::byte> body = bytes.subspan(at + kFrame, length);
        if ((next != 0 && (allow_gaps ? sequence < next : sequence != next)) ||
            sum != checksum(length, type, sequence, body)) {
            out.cut = true;
            break;
        }
        out.entries.push_back({type, sequence, Bytes(body.begin(), body.end())});
        at += kFrame + length;
        out.good = at;
        next = sequence + 1;
    }
    return out;
}

}  // namespace kd::save
