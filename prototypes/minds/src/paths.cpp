// P6's paths (A11): see paths.hpp. Pre-production code (research 00).
#include "paths.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>

namespace minds {

namespace {

constexpr float kStraight = static_cast<float>(kCellMetres);
constexpr float kDiagonal = static_cast<float>(kCellMetres * 1.41421356237309514547);
// the eight steps: four straight, then four diagonal
constexpr std::array<int, 8> kDx = {1, -1, 0, 0, 1, 1, -1, -1};
constexpr std::array<int, 8> kDy = {0, 0, 1, -1, 1, -1, 1, -1};
constexpr int kInCluster = kBlock * kBlock;
constexpr std::uint16_t kUnreached = 0xffffU;
// the shared cache starts again past this many paths
constexpr std::size_t kCacheMost = 200000;

// The least a trip between two cells can cost: the octile distance over open ground.
float octile(int a, int b) {
    const int dx = std::abs(cell_x(a) - cell_x(b));
    const int dy = std::abs(cell_y(a) - cell_y(b));
    const int lo = std::min(dx, dy);
    const int hi = std::max(dx, dy);
    return (static_cast<float>(hi - lo) * kStraight) + (static_cast<float>(lo) * kDiagonal);
}

using Heap = std::vector<std::pair<float, std::int32_t>>;

// A min-heap of (cost, id), ties broken by the smaller id.
void push(Heap* heap, float f, std::int32_t id) {
    heap->emplace_back(f, id);
    std::push_heap(heap->begin(), heap->end(), std::greater<>());
}

std::pair<float, std::int32_t> pop(Heap* heap) {
    std::pop_heap(heap->begin(), heap->end(), std::greater<>());
    const auto top = heap->back();
    heap->pop_back();
    return top;
}

// A cell's place inside its cluster.
int local(int cell) {
    return ((cell_y(cell) % kBlock) * kBlock) + (cell_x(cell) % kBlock);
}

std::uint64_t pair_key(std::int32_t a, std::int32_t b) {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a)) << 32U) | static_cast<std::uint32_t>(b);
}

template <typename A>
auto& at(A& a, std::int64_t i) {
    return a[static_cast<std::size_t>(i)];
}

}  // namespace

PathScratch::PathScratch(int entrances)
    : node_cost_(static_cast<std::size_t>(entrances) + 1, 0.0F),
      node_came_(static_cast<std::size_t>(entrances) + 1, -1),
      node_seen_(static_cast<std::size_t>(entrances) + 1, 0),
      node_closed_(static_cast<std::size_t>(entrances) + 1, 0),
      goal_cost_(static_cast<std::size_t>(entrances), 0.0F),
      goal_seen_(static_cast<std::size_t>(entrances), 0) {
    open_.reserve(1024);
}

bool Paths::step_ok(int from, int dir) const {
    const int x = cell_x(from) + at(kDx, dir);
    const int y = cell_y(from) + at(kDy, dir);
    if (x < 0 || y < 0 || x >= kSide || y >= kSide || !land_.passable(cell_at(x, y))) {
        return false;
    }
    // a diagonal step never cuts a corner of ground no one crosses
    return dir < 4 || (land_.passable(cell_at(x, cell_y(from))) && land_.passable(cell_at(cell_x(from), y)));
}

Paths::Paths(const Land& land)
    : land_(land), region_(static_cast<std::size_t>(kCells), -1), part_(static_cast<std::size_t>(kCells), -1) {
    std::vector<std::int32_t> stack;
    // Connected regions over the whole land, and each cluster's parts, by flood fill in cell order.
    std::int32_t regions = 0;
    for (int c = 0; c < kCells; ++c) {
        if (!land_.passable(c) || at(region_, c) >= 0) {
            continue;
        }
        at(region_, c) = regions;
        stack.push_back(c);
        while (!stack.empty()) {
            const int here = stack.back();
            stack.pop_back();
            for (int dir = 0; dir < 8; ++dir) {
                if (!step_ok(here, dir)) {
                    continue;
                }
                const int next = cell_at(cell_x(here) + at(kDx, dir), cell_y(here) + at(kDy, dir));
                if (at(region_, next) < 0) {
                    at(region_, next) = regions;
                    stack.push_back(next);
                }
            }
        }
        ++regions;
    }
    for (int c = 0; c < kCells; ++c) {
        if (!land_.passable(c) || at(part_, c) >= 0) {
            continue;
        }
        const auto id = static_cast<std::int32_t>(part_middle_.size());
        const int cluster = block_of(c);
        const int cx = ((cluster % kBlocks) * kBlock) + (kBlock / 2);
        const int cy = ((cluster / kBlocks) * kBlock) + (kBlock / 2);
        int middle = c;
        int nearest = kSide * kSide;
        at(part_, c) = id;
        stack.push_back(c);
        while (!stack.empty()) {
            const int here = stack.back();
            stack.pop_back();
            const int d = ((cell_x(here) - cx) * (cell_x(here) - cx)) + ((cell_y(here) - cy) * (cell_y(here) - cy));
            if (d < nearest || (d == nearest && here < middle)) {
                nearest = d;
                middle = here;
            }
            for (int dir = 0; dir < 8; ++dir) {
                if (!step_ok(here, dir)) {
                    continue;
                }
                const int next = cell_at(cell_x(here) + at(kDx, dir), cell_y(here) + at(kDy, dir));
                if (block_of(next) == cluster && at(part_, next) < 0) {
                    at(part_, next) = id;
                    stack.push_back(next);
                }
            }
        }
        part_middle_.push_back(middle);
        part_nodes_.emplace_back();
    }
    // Entrances: each run of open cell pairs across a border between two clusters gives one at its middle, or, if six
    // or longer, one at each end (Botea and others' HPA*). Each entrance is a node on either side.
    std::vector<std::int32_t> node_of(static_cast<std::size_t>(kCells), -1);
    auto node = [this, &node_of](int cell) {
        std::int32_t& n = at(node_of, cell);
        if (n < 0) {
            n = static_cast<std::int32_t>(node_cell_.size());
            node_cell_.push_back(cell);
            edges_.emplace_back();
            at(part_nodes_, at(part_, cell)).push_back(n);
        }
        return n;
    };
    auto join = [this, &node](int a, int b) {
        const std::int32_t na = node(a);
        const std::int32_t nb = node(b);
        at(edges_, na).push_back({nb, kStraight * land_.effort(b)});
        at(edges_, nb).push_back({na, kStraight * land_.effort(a)});
    };
    // across: true for the borders between a cluster and the next one east, false for the next one south
    for (const bool across : {true, false}) {
        const int beyond = across ? 1 : kSide;
        for (int line = kBlock - 1; line < kSide - 1; line += kBlock) {
            auto cell = [across, line](int k) { return across ? cell_at(line, k) : cell_at(k, line); };
            int run = -1;
            for (int along = 0; along <= kSide; ++along) {
                const bool open = along < kSide && land_.passable(cell(along)) && land_.passable(cell(along) + beyond);
                // a run ends at a closed pair, at the line's end, or where the clusters along the line change
                if (run >= 0 && (!open || along % kBlock == 0)) {
                    const int last = along - 1;
                    if (last - run + 1 >= 6) {
                        join(cell(run), cell(run) + beyond);
                        join(cell(last), cell(last) + beyond);
                    } else {
                        const int mid = (run + last) / 2;
                        join(cell(mid), cell(mid) + beyond);
                    }
                    run = -1;
                }
                if (open && run < 0) {
                    run = along;
                }
            }
        }
    }
    // From each entrance, its field of distances over its part, and so the costs between the part's entrances.
    fields_.assign(node_cell_.size() * kInCluster, kUnreached);
    PathScratch s;
    std::vector<float> f;
    for (std::size_t n = 0; n < node_cell_.size(); ++n) {
        flood(node_cell_[n], &s, &f);
        for (int k = 0; k < kInCluster; ++k) {
            if (at(f, k) >= 0.0F) {
                at(fields_, (static_cast<std::int64_t>(n) * kInCluster) + k) =
                    static_cast<std::uint16_t>(std::min(65534L, std::lround(at(f, k) * 10.0F)));
            }
        }
    }
    for (const auto& nodes : part_nodes_) {
        for (const std::int32_t a : nodes) {
            for (const std::int32_t b : nodes) {
                if (a != b) {
                    at(edges_, a).push_back({b, field(a, at(node_cell_, b))});
                }
            }
        }
    }
}

float Paths::field(std::int32_t node, int cell) const {
    const std::uint16_t v = at(fields_, (static_cast<std::int64_t>(node) * kInCluster) + local(cell));
    return v == kUnreached ? -1.0F : static_cast<float>(v) / 10.0F;
}

float Paths::search(int from, int to, PathScratch* s) const {
    const int cluster = block_of(from);
    const std::uint32_t id = ++s->search_;
    s->open_.clear();
    at(s->seen_, local(from)) = id;
    at(s->cost_, local(from)) = 0.0F;
    push(&s->open_, octile(from, to), from);
    while (!s->open_.empty()) {
        const auto [f, here] = pop(&s->open_);
        const int lh = local(here);
        if (at(s->closed_, lh) == id) {
            continue;
        }
        at(s->closed_, lh) = id;
        const float g = at(s->cost_, lh);
        if (here == to) {
            return g;
        }
        for (int dir = 0; dir < 8; ++dir) {
            if (!step_ok(here, dir)) {
                continue;
            }
            const int next = cell_at(cell_x(here) + at(kDx, dir), cell_y(here) + at(kDy, dir));
            if (block_of(next) != cluster) {
                continue;
            }
            const int ln = local(next);
            const float cost = g + ((dir < 4 ? kStraight : kDiagonal) * land_.effort(next));
            if (at(s->closed_, ln) != id && (at(s->seen_, ln) != id || cost < at(s->cost_, ln))) {
                at(s->seen_, ln) = id;
                at(s->cost_, ln) = cost;
                push(&s->open_, cost + octile(next, to), next);
            }
        }
    }
    return -1.0F;
}

void Paths::flood(int cell, PathScratch* s, std::vector<float>* field) const {
    field->assign(kInCluster, -1.0F);
    const int cluster = block_of(cell);
    const std::uint32_t id = ++s->search_;
    s->open_.clear();
    at(s->seen_, local(cell)) = id;
    at(s->cost_, local(cell)) = 0.0F;
    push(&s->open_, 0.0F, cell);
    while (!s->open_.empty()) {
        const auto [g, here] = pop(&s->open_);
        const int lh = local(here);
        if (at(s->closed_, lh) == id) {
            continue;
        }
        at(s->closed_, lh) = id;
        at(*field, lh) = g;
        for (int dir = 0; dir < 8; ++dir) {
            if (!step_ok(here, dir)) {
                continue;
            }
            const int next = cell_at(cell_x(here) + at(kDx, dir), cell_y(here) + at(kDy, dir));
            if (block_of(next) != cluster) {
                continue;
            }
            const int ln = local(next);
            const float cost = g + ((dir < 4 ? kStraight : kDiagonal) * land_.effort(next));
            if (at(s->closed_, ln) != id && (at(s->seen_, ln) != id || cost < at(s->cost_, ln))) {
                at(s->seen_, ln) = id;
                at(s->cost_, ln) = cost;
                push(&s->open_, cost, next);
            }
        }
    }
}

void Paths::relax(PathScratch* s, std::uint32_t id, std::int32_t from, std::int32_t to, float cost, float h) {
    if (at(s->node_closed_, to) != id && (at(s->node_seen_, to) != id || cost < at(s->node_cost_, to))) {
        at(s->node_seen_, to) = id;
        at(s->node_cost_, to) = cost;
        at(s->node_came_, to) = from;
        push(&s->open_, cost + h, to);
    }
}

bool Paths::abstract(const Reached& starts, const Reached& goals, int goal_cell, PathScratch* s, PartPath* out) const {
    const auto count = node_cell_.size();
    if (s->node_cost_.size() != count + 1) {
        return false;  // a scratch made for another land
    }
    const std::uint32_t id = ++s->search_;
    const auto goal = static_cast<std::int32_t>(count);  // the goal cell, as one more node
    for (const auto& [n, c] : goals) {
        at(s->goal_seen_, n) = id;
        at(s->goal_cost_, n) = c;
    }
    s->open_.clear();
    for (const auto& [n, c] : starts) {
        if (at(s->node_seen_, n) != id || c < at(s->node_cost_, n)) {
            at(s->node_seen_, n) = id;
            at(s->node_cost_, n) = c;
            at(s->node_came_, n) = -1;
            push(&s->open_, c + octile(at(node_cell_, n), goal_cell), n);
        }
    }
    while (!s->open_.empty()) {
        const auto [f, here] = pop(&s->open_);
        if (at(s->node_closed_, here) == id) {
            continue;
        }
        at(s->node_closed_, here) = id;
        const float g = at(s->node_cost_, here);
        if (here == goal) {
            out->via.clear();
            for (std::int32_t n = at(s->node_came_, here); n >= 0; n = at(s->node_came_, n)) {
                out->via.push_back(n);
            }
            std::reverse(out->via.begin(), out->via.end());
            if (out->via.empty()) {
                break;  // the goal is reached only from an entrance, so never so
            }
            out->metres = at(s->node_cost_, out->via.back()) - at(s->node_cost_, out->via.front());
            out->found = true;
            return true;
        }
        for (const Edge& e : at(edges_, here)) {
            relax(s, id, here, e.to, g + e.metres, octile(at(node_cell_, e.to), goal_cell));
        }
        if (at(s->goal_seen_, here) == id) {
            relax(s, id, here, goal, g + at(s->goal_cost_, here), 0.0F);
        }
    }
    out->found = false;
    return false;
}

bool Paths::trip(int from, int to, PathScratch* s, Trip* out) const {
    out->via.clear();
    out->metres = 0.0F;
    if (!land_.passable(from) || !land_.passable(to) || region(from) != region(to)) {
        ++s->counts.none;
        return false;
    }
    const std::int32_t pa = at(part_, from);
    const std::int32_t pb = at(part_, to);
    if (pa == pb) {
        const float m = search(from, to, s);
        if (m < 0.0F) {
            ++s->counts.none;
            return false;
        }
        out->metres = m;
        ++s->counts.inside;
        return true;
    }
    // the path between the two parts, from the cache shared by every thread, or this thread's own, or found now
    const std::uint64_t key = pair_key(pa, pb);
    const PartPath* path = nullptr;
    bool fresh = false;
    if (const auto it = cache_.find(key); it != cache_.end()) {
        path = &it->second;
    } else if (const auto mine = s->fresh_.find(key); mine != s->fresh_.end()) {
        path = &mine->second;
    } else {
        // from the two parts' middles, never the trip's own ends, so it is the same whoever finds it
        const int ma = at(part_middle_, pa);
        const int mb = at(part_middle_, pb);
        Reached starts;
        Reached goals;
        for (const std::int32_t n : at(part_nodes_, pa)) {
            const float f = field(n, ma);
            if (f >= 0.0F) {
                starts.emplace_back(n, f);
            }
        }
        for (const std::int32_t n : at(part_nodes_, pb)) {
            const float f = field(n, mb);
            if (f >= 0.0F) {
                goals.emplace_back(n, f);
            }
        }
        PartPath found;
        abstract(starts, goals, mb, s, &found);
        path = &s->fresh_.emplace(key, std::move(found)).first->second;
        fresh = true;
    }
    if (!path->found) {
        ++s->counts.none;
        return false;
    }
    // each end's distance from its field
    out->metres = field(path->via.front(), from) + path->metres + field(path->via.back(), to);
    for (const std::int32_t n : path->via) {
        out->via.push_back(at(node_cell_, n));
    }
    ++(fresh ? s->counts.fresh : s->counts.cached);
    return true;
}

void Paths::merge(PathScratch* s) {
    if (cache_.size() + s->fresh_.size() > kCacheMost) {
        cache_.clear();
    }
    for (auto& [key, path] : s->fresh_) {
        cache_.try_emplace(key, std::move(path));
    }
    s->fresh_.clear();
}

}  // namespace minds
