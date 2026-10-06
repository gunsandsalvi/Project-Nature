#include "kd/save/files.hpp"

#include <algorithm>
#include <cerrno>
#include <utility>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kd/core/check.hpp"

namespace kd::save {

std::string folder_of(const std::string& path) {
    const std::size_t slash = path.rfind('/');
    return slash == std::string::npos ? std::string() : path.substr(0, slash);
}

// --- the protocols, on the primitives

bool Files::write_whole(const std::string& path, std::span<const std::byte> bytes) {
    const std::string folder = folder_of(path);
    const std::string fresh = path + ".new";
    return make_folder(folder) && put(fresh, bytes) && sync_file(fresh) && move(fresh, path) && sync_folder(folder);
}

bool Files::append(const std::string& path, std::span<const std::byte> bytes) {
    return make_folder(folder_of(path)) && add(path, bytes);
}

bool Files::sync(const std::string& path) {
    // the file's bytes, then its name, which may be new
    return sync_file(path) && sync_folder(folder_of(path));
}

bool Files::cut(const std::string& path, std::uint64_t length) {
    return truncate(path, length) && sync_file(path);
}

bool Files::set_aside(const std::string& path) {
    return move(path, path + ".damaged") && sync_folder(folder_of(path));
}

bool Files::remove(const std::string& path) {
    return erase(path) && sync_folder(folder_of(path));
}

bool Files::rename(const std::string& from, const std::string& to) {
    return move(from, to) && sync_folder(folder_of(to));
}

// --- the disk

DiskFiles::DiskFiles(std::string root) : root_(std::move(root)) {}

std::string DiskFiles::full(const std::string& path) const {
    return path.empty() ? root_ : root_ + "/" + path;
}

std::optional<Bytes> DiskFiles::read(const std::string& path) {
    const int fd = ::open(full(path).c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return std::nullopt;
    }
    Bytes out;
    std::byte buffer[65536];
    for (;;) {
        const ssize_t n = ::read(fd, buffer, sizeof buffer);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            ::close(fd);
            return std::nullopt;
        }
        if (n == 0) {
            break;
        }
        out.insert(out.end(), buffer, buffer + n);
    }
    ::close(fd);
    return out;
}

std::vector<std::string> DiskFiles::list(const std::string& folder) {
    std::vector<std::string> out;
    DIR* dir = ::opendir(full(folder).c_str());
    if (dir == nullptr) {
        return out;
    }
    while (const dirent* e = ::readdir(dir)) {
        const std::string name = e->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        std::string path = folder;
        if (!path.empty()) {
            path += '/';
        }
        path += name;
        struct stat st {};
        if (::stat(full(path).c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            out.push_back(name);
        }
    }
    ::closedir(dir);
    std::stable_sort(out.begin(), out.end());
    return out;
}

bool DiskFiles::write_all(const std::string& path, std::span<const std::byte> bytes, int flags) {
    const int fd = ::open(full(path).c_str(), flags | O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
    if (fd < 0) {
        return false;
    }
    std::size_t done = 0;
    while (done < bytes.size()) {
        const ssize_t n = ::write(fd, bytes.data() + done, bytes.size() - done);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            ::close(fd);
            return false;
        }
        done += static_cast<std::size_t>(n);
    }
    return ::close(fd) == 0;
}

bool DiskFiles::put(const std::string& path, std::span<const std::byte> bytes) {
    return write_all(path, bytes, O_TRUNC);
}

bool DiskFiles::add(const std::string& path, std::span<const std::byte> bytes) {
    return write_all(path, bytes, O_APPEND);
}

bool DiskFiles::truncate(const std::string& path, std::uint64_t length) {
    return ::truncate(full(path).c_str(), static_cast<off_t>(length)) == 0;
}

bool DiskFiles::move(const std::string& from, const std::string& to) {
    return ::rename(full(from).c_str(), full(to).c_str()) == 0;
}

bool DiskFiles::erase(const std::string& path) {
    return ::unlink(full(path).c_str()) == 0 || errno == ENOENT;
}

bool DiskFiles::sync_file(const std::string& path) {
    const int fd = ::open(full(path).c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return false;
    }
    const bool synced = ::fsync(fd) == 0;
    return ::close(fd) == 0 && synced;
}

bool DiskFiles::sync_folder(const std::string& folder) {
    // a file's own fsync does not make its name durable; its folder's does (research 18)
    const int fd = ::open(full(folder).c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        return false;
    }
    const bool synced = ::fsync(fd) == 0;
    return ::close(fd) == 0 && synced;
}

bool DiskFiles::make_folder(const std::string& folder) {
    // each missing folder from the root down, its parent synced so its name lasts
    std::string made;
    std::size_t from = 0;
    while (from <= folder.size() && !folder.empty()) {
        const std::size_t slash = folder.find('/', from);
        const std::string part = folder.substr(from, slash == std::string::npos ? std::string::npos : slash - from);
        const std::string parent = made;
        if (!made.empty()) {
            made += '/';
        }
        made += part;
        if (::mkdir(full(made).c_str(), 0755) == 0) {
            if (!sync_folder(parent)) {
                return false;
            }
        } else if (errno != EEXIST) {
            return false;
        }
        if (slash == std::string::npos) {
            break;
        }
        from = slash + 1;
    }
    return true;
}

// --- the fake

bool FakeFiles::call() {
    ++calls_;
    if (calls_left_) {
        if (*calls_left_ == 0) {
            return false;
        }
        --*calls_left_;
    }
    return true;
}

std::optional<Bytes> FakeFiles::read(const std::string& path) {
    const auto at = names_.find(path);
    if (at == names_.end()) {
        return std::nullopt;
    }
    return nodes_[at->second].now;
}

std::vector<std::string> FakeFiles::list(const std::string& folder) {
    std::vector<std::string> out;
    for (const auto& [path, node] : names_) {
        if (folder_of(path) == folder) {
            out.push_back(folder.empty() ? path : path.substr(folder.size() + 1));
        }
    }
    return out;
}

bool FakeFiles::put(const std::string& path, std::span<const std::byte> bytes) {
    if (!call()) {
        return false;
    }
    const auto at = names_.find(path);
    if (at == names_.end()) {
        nodes_.push_back({Bytes(bytes.begin(), bytes.end()), {}});
        names_[path] = nodes_.size() - 1;
    } else {
        nodes_[at->second].now.assign(bytes.begin(), bytes.end());
    }
    return true;
}

bool FakeFiles::add(const std::string& path, std::span<const std::byte> bytes) {
    if (!call()) {
        return false;
    }
    const auto at = names_.find(path);
    if (at == names_.end()) {
        nodes_.push_back({Bytes(bytes.begin(), bytes.end()), {}});
        names_[path] = nodes_.size() - 1;
    } else {
        Bytes& now = nodes_[at->second].now;
        now.insert(now.end(), bytes.begin(), bytes.end());
    }
    return true;
}

bool FakeFiles::truncate(const std::string& path, std::uint64_t length) {
    const auto at = names_.find(path);
    if (!call() || at == names_.end()) {
        return false;
    }
    Bytes& now = nodes_[at->second].now;
    now.resize(std::min<std::uint64_t>(length, now.size()));
    return true;
}

bool FakeFiles::move(const std::string& from, const std::string& to) {
    const auto at = names_.find(from);
    if (!call() || at == names_.end()) {
        return false;
    }
    const std::size_t node = at->second;
    names_.erase(at);
    names_[to] = node;
    return true;
}

bool FakeFiles::erase(const std::string& path) {
    if (!call()) {
        return false;
    }
    names_.erase(path);
    return true;
}

bool FakeFiles::sync_file(const std::string& path) {
    const auto at = names_.find(path);
    if (!call() || at == names_.end()) {
        return false;
    }
    Node& n = nodes_[at->second];
    n.safe = n.now;
    return true;
}

bool FakeFiles::sync_folder(const std::string& folder) {
    if (!call()) {
        return false;
    }
    std::erase_if(safe_names_, [&](const auto& entry) { return folder_of(entry.first) == folder; });
    for (const auto& [path, node] : names_) {
        if (folder_of(path) == folder) {
            safe_names_[path] = node;
        }
    }
    return true;
}

bool FakeFiles::make_folder(const std::string& /*folder*/) {
    return call();
}

void FakeFiles::power_cut() {
    names_ = safe_names_;
    for (Node& n : nodes_) {
        n.now = n.safe;
    }
}

Bytes& FakeFiles::raw(const std::string& path) {
    const auto at = names_.find(path);
    KD_CHECK(at != names_.end(), "save::FakeFiles: no such file to damage");
    return nodes_[at->second].now;
}

// --- the I/O thread

IoThread::IoThread(Files& files) : files_(files) {
    thread_ = std::make_unique<run::Thread>("kd-io", [this] { loop(); });
}

IoThread::~IoThread() {
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    thread_.reset();
}

void IoThread::post(std::function<void(Files&)> job) {
    {
        std::lock_guard lock(mutex_);
        jobs_.push_back(std::move(job));
        ++posted_;
    }
    wake_.notify_all();
}

void IoThread::flush() {
    std::unique_lock lock(mutex_);
    const std::uint64_t target = posted_;
    done_.wait(lock, [&] { return ran_ >= target; });
}

void IoThread::now(const std::function<void(Files&)>& job) {
    post([&job](Files& f) { job(f); });
    flush();
}

void IoThread::loop() {
    std::unique_lock lock(mutex_);
    for (;;) {
        wake_.wait(lock, [&] { return stopping_ || !jobs_.empty(); });
        if (jobs_.empty()) {
            return;
        }
        std::function<void(Files&)> job = std::move(jobs_.front());
        jobs_.pop_front();
        lock.unlock();
        job(files_);
        lock.lock();
        ++ran_;
        done_.notify_all();
    }
}

}  // namespace kd::save
