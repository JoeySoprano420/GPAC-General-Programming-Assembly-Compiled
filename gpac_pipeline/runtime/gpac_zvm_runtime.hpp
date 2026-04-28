#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace gpac::zvm {

struct Runtime {
    std::mutex io_lock;

    void print(const std::string& value) {
        std::lock_guard<std::mutex> lock(io_lock);
        std::cout << value << std::endl;
    }

    void print(int value) {
        std::lock_guard<std::mutex> lock(io_lock);
        std::cout << value << std::endl;
    }

    void print(double value) {
        std::lock_guard<std::mutex> lock(io_lock);
        std::cout << value << std::endl;
    }

    void print(bool value) {
        std::lock_guard<std::mutex> lock(io_lock);
        std::cout << (value ? "true" : "false") << std::endl;
    }

    std::string input() {
        std::string s;
        std::getline(std::cin, s);
        return s;
    }

    template <typename Fn>
    void run_track(Fn&& fn) {
        std::thread t(std::forward<Fn>(fn));
        t.join();
    }
};

inline int native_add(int a, int b) {
    return a + b;
}

} // namespace gpac::zvm
