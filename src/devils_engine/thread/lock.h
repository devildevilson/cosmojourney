#ifndef DEVILS_ENGINE_THREAD_LOCK_H
#define DEVILS_ENGINE_THREAD_LOCK_H

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <thread>

namespace devils_engine {
namespace thread {
class spin_mutex {
public: 
  inline spin_mutex() noexcept : val(false) {}
  inline bool try_lock() noexcept { return !val.exchange(true); }
  inline void lock() noexcept { while (val.exchange(true)) {} }
  inline void unlock() noexcept { val = false; }
private:
  std::atomic_bool val;
};

class light_spin_mutex {
public: 
  inline light_spin_mutex() noexcept : val(false) {}
  inline bool try_lock() noexcept { return !val.exchange(true); }
  inline void lock() noexcept { while (val.exchange(true)) { std::this_thread::sleep_for(std::chrono::nanoseconds(1)); } }
  inline void unlock() noexcept { val = false; }
private:
  std::atomic_bool val;
};

template <typename T>
void spin_sleep_for(const T &dur) {
  const size_t amount = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  const auto tp = std::chrono::high_resolution_clock::now();
  size_t ns = 0;
  while (ns < amount) {
    const auto new_dur = std::chrono::high_resolution_clock::now() - tp;
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(new_dur).count();
  }
}

template <typename T>
void spin_sleep_until(const T &tp) {
  const auto orig_tp = std::chrono::clock_cast<std::chrono::high_resolution_clock>(tp);
  int64_t ns = -1;
  while (ns < 0) {
    const auto new_dur = std::chrono::high_resolution_clock::now() - orig_tp;
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(new_dur).count();
  }
}

template <typename T>
void light_spin_sleep_for(const T &dur) {
  const size_t amount = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  const auto tp = std::chrono::high_resolution_clock::now();
  size_t ns = 0;
  while (ns < amount) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    const auto new_dur = std::chrono::high_resolution_clock::now() - tp;
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(new_dur).count();
  }
}

template <typename T>
void light_spin_sleep_until(const T &tp) {
  const auto orig_tp = std::chrono::clock_cast<std::chrono::high_resolution_clock>(tp);
  int64_t ns = -1;
  while (ns < 0) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    const auto new_dur = std::chrono::high_resolution_clock::now() - orig_tp;
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(new_dur).count();
  }
}
}
}

#endif