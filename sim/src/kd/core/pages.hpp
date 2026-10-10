// Immutable record pages and a bounded writable tail (A19.2). Copies share sealed
// pages; record identities and logical iteration do not depend on page boundaries.
#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <vector>

#include "kd/core/bytes.hpp"
#include "kd/core/check.hpp"

namespace kd {
// Disposable sparse serial lookup. A publication shares unchanged blocks; the
// producer copies only a block it subsequently changes. No record payload copies.
class RecordIndex {
public:
    static constexpr std::size_t kBlock = 512;
    static constexpr std::size_t kMissing = static_cast<std::size_t>(-1);
    using Block = std::array<std::size_t, kBlock>;
    [[nodiscard]] std::size_t get(std::uint64_t serial) const {
        const auto block = serial / kBlock;
        const auto at = std::lower_bound(blocks_.begin(), blocks_.end(), block,
                                         [](const auto& entry, std::uint64_t wanted) { return entry.first < wanted; });
        return at == blocks_.end() || at->first != block ? kMissing : (*at->second)[serial % kBlock];
    }
    void add(std::uint64_t serial, std::size_t record) {
        const auto block = serial / kBlock;
        auto at = std::lower_bound(blocks_.begin(), blocks_.end(), block,
                                   [](const auto& entry, std::uint64_t wanted) { return entry.first < wanted; });
        if (at == blocks_.end() || at->first != block) {
            at = blocks_.insert(at, {block, std::make_shared<Block>()});
            at->second->fill(kMissing);
        } else if (!at->second.unique())
            at->second = std::make_shared<Block>(*at->second);
        KD_CHECK((*at->second)[serial % kBlock] == kMissing, "an archived serial cannot be reused");
        (*at->second)[serial % kBlock] = record;
    }

private:
    std::vector<std::pair<std::uint64_t, std::shared_ptr<Block>>> blocks_;
};

template <typename T>
class Pages {
public:
    using value_type = T;
    using Page = std::shared_ptr<const std::vector<T>>;
    Pages() = default;
    Pages(std::initializer_list<T> values) {
        for (const auto& value : values) push_back(value);
    }
    static constexpr std::size_t kPage = 512;
    class Iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = const T&;
        using pointer = const T*;
        using iterator_category = std::random_access_iterator_tag;
        Iterator() = default;
        Iterator(const Pages* owner, difference_type at) : owner_(owner), at_(at) {}
        reference operator*() const { return (*owner_)[static_cast<std::size_t>(at_)]; }
        pointer operator->() const { return &**this; }
        reference operator[](difference_type n) const { return *(*this + n); }
        Iterator& operator++() {
            ++at_;
            return *this;
        }
        Iterator operator++(int) {
            auto before = *this;
            ++*this;
            return before;
        }
        Iterator& operator--() {
            --at_;
            return *this;
        }
        Iterator operator--(int) {
            auto before = *this;
            --*this;
            return before;
        }
        Iterator& operator+=(difference_type n) {
            at_ += n;
            return *this;
        }
        Iterator& operator-=(difference_type n) {
            at_ -= n;
            return *this;
        }
        friend Iterator operator+(Iterator i, difference_type n) { return i += n; }
        friend Iterator operator+(difference_type n, Iterator i) { return i += n; }
        friend Iterator operator-(Iterator i, difference_type n) { return i -= n; }
        friend difference_type operator-(Iterator a, Iterator b) { return a.at_ - b.at_; }
        friend auto operator<=>(const Iterator&, const Iterator&) = default;

    private:
        const Pages* owner_ = nullptr;
        difference_type at_ = 0;
    };
    [[nodiscard]] std::size_t size() const { return sealed_ + tail_.size(); }
    [[nodiscard]] bool empty() const { return size() == 0; }
    [[nodiscard]] Iterator begin() const { return {this, 0}; }
    [[nodiscard]] Iterator end() const { return {this, static_cast<std::ptrdiff_t>(size())}; }
    [[nodiscard]] auto rbegin() const { return std::reverse_iterator(end()); }
    [[nodiscard]] auto rend() const { return std::reverse_iterator(begin()); }
    [[nodiscard]] const T& back() const { return (*this)[size() - 1]; }
    [[nodiscard]] const T& front() const { return (*this)[0]; }
    [[nodiscard]] const T& operator[](std::size_t n) const {
        KD_CHECK(n < size(), "record page index outside retained records");
        if (n >= sealed_) return tail_[n - sealed_];
        const auto p = static_cast<std::size_t>(std::upper_bound(ends_.begin(), ends_.end(), n) - ends_.begin());
        return (*pages_[p])[n - (p == 0 ? 0 : ends_[p - 1])];
    }
    // Only newly appended records may still be completed by their producing event.
    [[nodiscard]] T& writable(std::size_t n) {
        KD_CHECK(n >= sealed_ && n < size(), "a sealed record is immutable");
        return tail_[n - sealed_];
    }
    void push_back(T value) {
        if (tail_.size() == kPage) seal();
        tail_.push_back(std::move(value));
    }
    void reserve(std::size_t n) { tail_.reserve(std::min(n, kPage)); }
    void clear() {
        pages_.clear();
        ends_.clear();
        encoded_.clear();
        tail_.clear();
        sealed_ = 0;
    }
    void seal() {
        if (tail_.empty()) return;
        append_page(std::make_shared<const std::vector<T>>(std::move(tail_)));
        tail_ = {};
    }
    [[nodiscard]] const std::vector<Page>& pages() const { return pages_; }
    [[nodiscard]] const std::vector<T>& tail() const { return tail_; }
    void append_page(Page page, std::shared_ptr<const std::vector<std::byte>> encoded = {}) {
        KD_CHECK(tail_.empty() && !page->empty() && page->size() <= kPage, "invalid immutable record page");
        sealed_ += page->size();
        pages_.push_back(std::move(page));
        ends_.push_back(sealed_);
        encoded_.push_back(std::move(encoded));
    }
    // Display trails discard only the prefix passed by the screen. Retained sealed
    // pages stay shared with earlier publications; at most one boundary page changes.
    void drop_prefix(std::size_t count) {
        KD_CHECK(count <= size(), "prefix outside retained records");
        if (count == 0) return;
        Pages next;
        for (std::size_t n = 0; n < pages_.size(); ++n) {
            const auto& page = pages_[n];
            if (count >= page->size()) {
                count -= page->size();
                continue;
            }
            if (count) {
                next.append_page(std::make_shared<const std::vector<T>>(
                    page->begin() + static_cast<std::ptrdiff_t>(count), page->end()));
                count = 0;
            } else
                next.append_page(page, encoded_[n]);
        }
        next.tail_.assign(tail_.begin() + static_cast<std::ptrdiff_t>(count), tail_.end());
        *this = std::move(next);
    }
    template <typename Predicate>
    void retain(Predicate keep) {
        Pages next;
        for (std::size_t n = 0; n < pages_.size(); ++n) {
            const auto& page = pages_[n];
            if (std::all_of(page->begin(), page->end(), keep))
                next.append_page(page, encoded_[n]);
            else {
                std::vector<T> kept;
                for (const auto& value : *page)
                    if (keep(value)) kept.push_back(value);
                if (!kept.empty()) next.append_page(std::make_shared<const std::vector<T>>(std::move(kept)));
            }
        }
        for (const auto& value : tail_)
            if (keep(value)) next.tail_.push_back(value);
        *this = std::move(next);
    }
    template <typename Encode>
    [[nodiscard]] std::shared_ptr<const std::vector<std::byte>> encoded(std::size_t n, Encode encode) const {
        if (!encoded_[n]) encoded_[n] = std::make_shared<const std::vector<std::byte>>(encode(*pages_[n]));
        return encoded_[n];
    }
    template <typename U = T>
    [[nodiscard]] const T* find(decltype(U{}.id) id) const {
        using Id = decltype(U{}.id);
        const auto at =
            std::lower_bound(begin(), end(), id, [](const T& value, Id wanted) { return value.id < wanted; });
        return at != end() && at->id == id ? &*at : nullptr;
    }

private:
    std::vector<Page> pages_;
    std::vector<std::size_t> ends_;
    mutable std::vector<std::shared_ptr<const std::vector<std::byte>>> encoded_;
    std::vector<T> tail_;
    std::size_t sealed_ = 0;
};
}  // namespace kd
