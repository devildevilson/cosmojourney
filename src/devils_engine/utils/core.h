#ifndef DEVILS_ENGINE_UTILS_CORE_H
#define DEVILS_ENGINE_UTILS_CORE_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <stdexcept>
#include <chrono>
#include <iostream>
#include <source_location>
#include "spdlog/spdlog.h"

namespace devils_engine {
  namespace utils {
#   ifdef _WIN32
#     define utils_pretty_function __FUNCSIG__
#   else
#     define utils_pretty_function __PRETTY_FUNCTION__
#   endif

    std::string_view make_sane_file_name(const std::string_view &str);

    template <typename... Args>
    void error(const fmt::format_string<Args...> &format, Args&&... args) {
      spdlog::error(format, std::forward<Args>(args)...);
      throw std::runtime_error("Got runtime error");
    }

    template <typename... Args>
    void info(const fmt::format_string<Args...> &format, Args&&... args) {
      spdlog::info(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(const fmt::format_string<Args...> &format, Args&&... args) {
      spdlog::warn(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void assertf(const bool cond, const fmt::format_string<Args...> &format, Args&&... args) {
      if (!cond) {
        spdlog::error(format, std::forward<Args>(args)...);
        throw std::runtime_error("Assertion failed");
      }
    }

    template <typename... Args>
    void assertf_failed_detail(
      const std::string_view &cond_str,
      const std::string_view &file_name,
      const std::string_view &func_name,
      const size_t line,
      const fmt::format_string<Args...> &format,
      Args&&... args
    ) {
      spdlog::error("{}:{}: {}: Assertion `{}` failed", make_sane_file_name(file_name), line, func_name, cond_str);
      spdlog::info(format, std::forward<Args>(args)...);
      throw std::runtime_error("Assertion failed");
    }

    void assert_failed_detail(
      const std::string_view &cond_str,
      const std::string_view &file_name,
      const std::string_view &func_name,
      const size_t line
    );

    void assert_failed_detail(
      const std::string_view &cond_str,
      const std::string_view &file_name,
      const std::string_view &func_name,
      const size_t line,
      const std::string_view &comm
    );

#   define utils_assertf(cond, fmt, ...) {if (!(cond)) {devils_engine::utils::assertf_failed_detail(#cond, __FILE__, utils_pretty_function, __LINE__, fmt, __VA_ARGS__);}}
#   define utils_assert(cond) {if (!(cond)) {devils_engine::utils::assert_failed_detail(#cond, __FILE__, utils_pretty_function, __LINE__);}}
#   define utils_assertc(cond, comm) {if (!(cond)) {devils_engine::utils::assert_failed_detail(#cond, __FILE__, utils_pretty_function, __LINE__, comm);}}

    template <typename T>
    constexpr const T& max(const T &a) { return a; }

    template <typename T, typename... Ts>
    constexpr const T& max(const T &a, const T &b, Ts&&... args) {
      const T& m = max(b, std::forward<Ts>(args)...);
      return a >= m ? a : m;
    }

    template <typename T>
    constexpr const T& min(const T &a) { return a; }

    template <typename T, typename... Ts>
    constexpr const T& min(const T &a, const T &b, Ts&&... args) {
      const T& m = max(b, std::forward<Ts>(args)...);
      return a < m ? a : m;
    }

    constexpr size_t align_to(const size_t size, const size_t alignment) {
      return (size + alignment - 1) / alignment * alignment;
    }

    inline void print_detail() {}

    template <typename Arg, typename... Args>
    void print_detail(Arg&& arg, Args&&... args) {
      std::cout << " " << arg;
      print_detail(std::forward<Args>(args)...);
    }

    template <typename Arg, typename... Args>
    void print(Arg&& arg, Args&&... args) {
      std::cout << arg;
      print_detail(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void println(Args&&... args) {
      print(std::forward<Args>(args)...);
      std::cout << "\n";
    }

    class time_log {
    public:
      std::string str;
      std::chrono::steady_clock::time_point tp;

      time_log(std::string str) noexcept;
      ~time_log() noexcept;

      time_log(const time_log &copy) noexcept = delete;
      time_log(time_log &&move) noexcept = delete;
      time_log & operator=(const time_log &copy) noexcept = delete;
      time_log & operator=(time_log &&move) noexcept = delete;
    };

    class tracer {
    public:
      std::source_location l;

      tracer(std::source_location loc = std::source_location::current()) noexcept;
      ~tracer() noexcept;

      tracer(const tracer &copy) noexcept = delete;
      tracer(tracer &&move) noexcept = delete;
      tracer & operator=(const tracer &copy) noexcept = delete;
      tracer & operator=(tracer &&move) noexcept = delete;
    };
  }
}

#endif