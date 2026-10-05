#include "kd/run/thread.hpp"

#include <sys/resource.h>
#include <unistd.h>

#include <utility>

#include "kd/core/check.hpp"
#include "kd/num/fenv.hpp"

namespace kd::run {

namespace {

constexpr std::size_t kStackBytes = std::size_t{8} << 20;
constexpr int kNice = 2;  // one step below Godot's main thread, so it wins any core the two share (PRN-11)

}  // namespace

Thread::Thread(std::string name, std::function<void()> body) : name_(std::move(name)), body_(std::move(body)) {
    pthread_attr_t attr;
    KD_CHECK(pthread_attr_init(&attr) == 0, "could not prepare a simulation thread");
    KD_CHECK(pthread_attr_setstacksize(&attr, kStackBytes) == 0, "could not size a simulation thread's stack");
    KD_CHECK(pthread_create(&handle_, &attr, &Thread::start, this) == 0, "could not start a simulation thread");
    pthread_attr_destroy(&attr);
    ready_.wait(false);
}

Thread::~Thread() {
    pthread_join(handle_, nullptr);
}

void* Thread::start(void* self) {
    Thread& t = *static_cast<Thread*>(self);
    pthread_setname_np(pthread_self(), t.name_.substr(0, 15).c_str());
    setpriority(PRIO_PROCESS, static_cast<id_t>(gettid()), kNice);
    t.inherited_fenv_ = num::fenv_read();
    num::fenv_reset();
    num::fenv_assert_default();
    pthread_attr_t attr;
    if (pthread_getattr_np(pthread_self(), &attr) == 0) {
        pthread_attr_getstacksize(&attr, &t.stack_);
        pthread_attr_destroy(&attr);
    }
    t.ready_.store(true);
    t.ready_.notify_all();
    t.body_();
    return nullptr;
}

}  // namespace kd::run
