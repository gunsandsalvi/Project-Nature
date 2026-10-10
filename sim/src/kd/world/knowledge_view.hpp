// Exact, immutable display knowledge (A19.2). Scalar changes never duplicate
// unchanged personal evidence. Materialise a component only when it is inspected.
#pragma once
#include <algorithm>
#include <atomic>
#include <bit>
#include <memory>
#include <mutex>
#include <set>
#include <type_traits>
#include "kd/ecs/component.hpp"
#include "kd/world/knowledge.hpp"
namespace kd::world {
namespace knowledge_view_detail {
// The component descriptor copies every scalar automatically. Record payloads
// travel separately, so a new scalar cannot silently disappear from past views.
struct Scalars {
    std::vector<std::uint64_t>& values;
    bool reading;
    std::size_t at = 0;
    template <typename T>
    void integer(T& value) {
        if constexpr (std::is_const_v<T>)
            values.push_back(static_cast<std::uint64_t>(value));
        else if (reading) {
            KD_CHECK(at < values.size(), "display scalar outside captured descriptor");
            if constexpr (std::is_same_v<T, std::int64_t>)
                value = std::bit_cast<std::int64_t>(values[at++]);
            else
                value = static_cast<T>(values[at++]);
        } else
            values.push_back(static_cast<std::uint64_t>(value));
    }
    template <typename T>
    void u8(ecs::Part, T& v) {
        integer(v);
    }
    template <typename T>
    void u32(ecs::Part, T& v) {
        integer(v);
    }
    template <typename T>
    void u64(ecs::Part, T& v) {
        integer(v);
    }
    template <typename T>
    void i64(ecs::Part, T& v) {
        integer(v);
    }
    template <typename T>
    void id(ecs::Part, T& v) {
        integer(v.value);
    }
    template <typename T>
    void entry(ecs::Part, T& v, std::string_view) {
        integer(v);
    }
    template <typename T>
    void records(ecs::Part, T&, std::size_t, std::size_t = 4) {}
};
template <typename T>
using Records = std::vector<std::shared_ptr<const T>>;
template <typename T>
Records<T> capture_records(const std::vector<T>& source, const Records<T>& previous) {
    Records<T> out;
    out.reserve(source.size());
    const bool sorted = [&] {
        if constexpr (std::is_same_v<T, Memory>)
            return std::is_sorted(previous.begin(), previous.end(),
                                  [](const auto& a, const auto& b) { return a->id < b->id; });
        else
            return false;
    }();
    for (std::size_t n = 0; n < source.size(); ++n) {
        auto old = n < previous.size() ? previous[n] : nullptr;
        if constexpr (std::is_same_v<T, Memory>) {
            if (!old || old->id != source[n].id) {
                const auto at = sorted ? std::lower_bound(previous.begin(), previous.end(), source[n].id,
                                                          [](const auto& record, auto id) { return record->id < id; })
                                       : std::find_if(previous.begin(), previous.end(),
                                                      [&](const auto& record) { return record->id == source[n].id; });
                old = at != previous.end() && (*at)->id == source[n].id ? *at : nullptr;
            }
        }
        out.push_back(old && *old == source[n] ? old : std::make_shared<const T>(source[n]));
    }
    return out;
}
template <typename T>
void restore_records(std::vector<T>& destination, const Records<T>& records) {
    destination.reserve(records.size());
    for (const auto& value : records) destination.push_back(*value);
}
template <typename T>
std::size_t payload_bytes(const T& record) {
    std::size_t bytes = sizeof(T);
    if constexpr (requires { record.inputs; })
        bytes += record.inputs.capacity() * sizeof(typename std::decay_t<decltype(record.inputs)>::value_type);
    if constexpr (requires { record.participants; }) bytes += record.participants.capacity() * sizeof(Link);
    return bytes;
}
}  // namespace knowledge_view_detail
class KnowledgeView {
    static_assert(Knowledge::version == 3, "Review display record groups when knowledge changes");
    struct Data {
        std::vector<std::uint64_t> scalars;
        knowledge_view_detail::Records<Familiar> familiar;
        knowledge_view_detail::Records<Skill> skills;
        knowledge_view_detail::Records<Memory> memories;
        knowledge_view_detail::Records<Hunch> hunches;
        knowledge_view_detail::Records<CraftReason> reasons;
        knowledge_view_detail::Records<PeerBelief> peers;
        knowledge_view_detail::Records<Observation> observations;
        mutable std::once_flag inspected;
        mutable std::shared_ptr<const Knowledge> component_owner;
        mutable std::atomic<const Knowledge*> component{nullptr};
    };

public:
    KnowledgeView() = default;
    [[nodiscard]] static KnowledgeView capture(const Knowledge& source) { return capture(source, KnowledgeView{}); }
    [[nodiscard]] static KnowledgeView capture(const Knowledge& source, const KnowledgeView& before) {
        auto next = std::make_shared<Data>();
        knowledge_view_detail::Scalars scalars{next->scalars, false};
        Knowledge::visit(scalars, source);
        const Data empty;
        const auto& old = before.data_ ? *before.data_ : empty;
        next->familiar = knowledge_view_detail::capture_records(source.familiar, old.familiar);
        next->skills = knowledge_view_detail::capture_records(source.skills, old.skills);
        next->memories = knowledge_view_detail::capture_records(source.memories, old.memories);
        next->hunches = knowledge_view_detail::capture_records(source.hunches, old.hunches);
        next->reasons = knowledge_view_detail::capture_records(source.reasons, old.reasons);
        next->peers = knowledge_view_detail::capture_records(source.peers, old.peers);
        next->observations = knowledge_view_detail::capture_records(source.observations, old.observations);
        if (before.data_ && next->scalars == old.scalars && next->familiar == old.familiar &&
            next->skills == old.skills && next->memories == old.memories && next->hunches == old.hunches &&
            next->reasons == old.reasons && next->peers == old.peers && next->observations == old.observations)
            return before;
        KnowledgeView out;
        out.data_ = std::move(next);
        return out;
    }
    [[nodiscard]] explicit operator bool() const { return static_cast<bool>(data_); }
    [[nodiscard]] const Knowledge* operator->() const { return get(); }
    [[nodiscard]] const Knowledge* get() const {
        if (!data_) return nullptr;
        std::call_once(data_->inspected, [&] {
            Knowledge value;
            // Reading the descriptor does not alter captured scalar payloads.
            auto scalars = data_->scalars;
            knowledge_view_detail::Scalars reader{scalars, true};
            Knowledge::visit(reader, value);
            KD_CHECK(reader.at == scalars.size(), "display scalar descriptor changed");
            knowledge_view_detail::restore_records(value.familiar, data_->familiar);
            knowledge_view_detail::restore_records(value.skills, data_->skills);
            knowledge_view_detail::restore_records(value.memories, data_->memories);
            knowledge_view_detail::restore_records(value.hunches, data_->hunches);
            knowledge_view_detail::restore_records(value.reasons, data_->reasons);
            knowledge_view_detail::restore_records(value.peers, data_->peers);
            knowledge_view_detail::restore_records(value.observations, data_->observations);
            data_->component_owner = std::make_shared<const Knowledge>(std::move(value));
            data_->component.store(data_->component_owner.get(), std::memory_order_release);
        });
        return data_->component.load(std::memory_order_acquire);
    }
    [[nodiscard]] const void* identity() const { return data_.get(); }
    friend bool operator==(const KnowledgeView&, const KnowledgeView&) = default;
    // Actual retained payload estimate, deduplicated over a whole publication.
    // This does not inspect/materialise records and cannot affect the world.
    [[nodiscard]] std::size_t retained_bytes(std::set<const void*>& seen) const {
        if (!data_ || !seen.insert(data_.get()).second) return 0;
        std::size_t bytes = sizeof(Data) + data_->scalars.capacity() * sizeof(std::uint64_t);
        const auto count = [&](const auto& rows) {
            bytes += rows.capacity() * sizeof(typename std::decay_t<decltype(rows)>::value_type);
            for (const auto& row : rows)
                if (seen.insert(row.get()).second) bytes += knowledge_view_detail::payload_bytes(*row);
        };
        count(data_->familiar);
        count(data_->skills);
        count(data_->memories);
        count(data_->hunches);
        count(data_->reasons);
        count(data_->peers);
        count(data_->observations);
        if (const auto component = data_->component.load(std::memory_order_acquire)) {
            const auto& value = *component;
            bytes += sizeof(Knowledge);
            const auto copied = [&](const auto& rows) {
                using T = typename std::decay_t<decltype(rows)>::value_type;
                bytes += rows.capacity() * sizeof(T);
                for (const auto& row : rows) bytes += knowledge_view_detail::payload_bytes(row) - sizeof(T);
            };
            copied(value.familiar);
            copied(value.skills);
            copied(value.memories);
            copied(value.hunches);
            copied(value.reasons);
            copied(value.peers);
            copied(value.observations);
        }
        return bytes;
    }

private:
    std::shared_ptr<const Data> data_;
};
}  // namespace kd::world
