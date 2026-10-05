#include "kd/run/workers.hpp"

#include <sys/resource.h>
#include <unistd.h>

#include <cstdio>

#include "kd/core/check.hpp"
#include "kd/num/fenv.hpp"

namespace kd::run {

namespace {

constexpr std::size_t kStackBytes = std::size_t{8} << 20;
constexpr int kNice = 2;  // one step below Godot's main thread, so it wins any core the two share (PRN-11)

}  // namespace

Workers::Workers(int count, std::string name) : name_(std::move(name)) {
    KD_CHECK(count >= 1 && count <= 16, "the simulation runs on 1 to 16 threads");
    threads_.resize(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        Thread& t = threads_[static_cast<std::size_t>(i)];
        t.owner = this;
        t.index = i;
        pthread_attr_t attr;
        KD_CHECK(pthread_attr_init(&attr) == 0, "could not prepare a simulation thread");
        KD_CHECK(pthread_attr_setstacksize(&attr, kStackBytes) == 0, "could not size a simulation thread's stack");
        KD_CHECK(pthread_create(&t.handle, &attr, &Workers::start, &t) == 0, "could not start a simulation thread");
        pthread_attr_destroy(&attr);
    }
    std::unique_lock lock(mutex_);
    done_.wait(lock, [&] { return started_ == count; });
}

Workers::~Workers() {
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    for (Thread& t : threads_) {
        pthread_join(t.handle, nullptr);
    }
}

void* Workers::start(void* self) {
    Thread& t = *static_cast<Thread*>(self);
    char name[16];
    std::snprintf(name, sizeof name, "%s-%d", t.owner->name_.c_str(), t.index);
    pthread_setname_np(pthread_self(), name);
    setpriority(PRIO_PROCESS, static_cast<id_t>(gettid()), kNice);
    t.inherited_fenv = num::fenv_read();
    num::fenv_reset();
    num::fenv_assert_default();
    pthread_attr_t attr;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        pthread_attr_getstacksize(&attr, &t.stack);
        pthread_attr_destroy(&attr);
    }
    {
        std::lock_guard lock(t.owner->mutex_);
        ++t.owner->started_;
    }
    t.owner->done_.notify_all();
    t.owner->loop(t.index);
    return nullptr;
}

void Workers::loop(int /*index*/) {
    std::uint64_t seen = 0;
    for (;;) {
        Call call = nullptr;
        void* task = nullptr;
        std::size_t total = 0;
        {
            std::unique_lock lock(mutex_);
            wake_.wait(lock, [&] { return stopping_ || generation_ != seen; });
            if (stopping_) {
                return;
            }
            seen = generation_;
            call = call_;
            task = task_;
            total = total_;
        }
        num::fenv_assert_default();
        for (std::size_t i = next_.fetch_add(1); i < total; i = next_.fetch_add(1)) {
            call(task, i);
        }
        {
            std::lock_guard lock(mutex_);
            --busy_;
        }
        done_.notify_all();
    }
}

void Workers::run(std::size_t n, Call call, void* task) {
    if (n == 0) {
        return;
    }
    {
        std::lock_guard lock(mutex_);
        call_ = call;
        task_ = task;
        total_ = n;
        next_.store(0);
        busy_ = count();
        ++generation_;
    }
    wake_.notify_all();
    std::unique_lock lock(mutex_);
    done_.wait(lock, [&] { return busy_ == 0; });
}

std::vector<std::uint64_t> Workers::inherited_fenv() const {
    std::vector<std::uint64_t> out;
    out.reserve(threads_.size());
    for (const Thread& t : threads_) {
        out.push_back(t.inherited_fenv);
    }
    return out;
}

std::vector<std::size_t> Workers::stack_sizes() const {
    std::vector<std::size_t> out;
    out.reserve(threads_.size());
    for (const Thread& t : threads_) {
        out.push_back(t.stack);
    }
    return out;
}

}  // namespace kd::run
