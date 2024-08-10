#ifndef DEVILS_ENGINE_UTILS_CORE_H
#define DEVILS_ENGINE_UTILS_CORE_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <stdexcept>
#include <iostream>
#include <source_location>
#include <reflect>
#include "spdlog/spdlog.h"
#include "type_traits.h"

namespace devils_engine {
  namespace utils {
#   ifdef _MSC_VER
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

    template <class CharT, class Traits, typename T, size_t level = 1>
    std::basic_ostream<CharT, Traits> &print_part(std::basic_ostream<CharT, Traits> &os, const T &obj);

    template <class CharT, class Traits, typename T, size_t level>
    std::basic_ostream<CharT, Traits> &print_value(std::basic_ostream<CharT, Traits> &os, const T &val) {
      using mem_type = std::remove_cvref_t<T>;
      if constexpr (std::is_same_v<mem_type, bool>) {
        os << val ? "true" : "false";
      } else if constexpr (std::is_arithmetic_v<mem_type>) {
        os << val;
      } else if constexpr (std::is_same_v<mem_type, const char *> ||
                           std::is_same_v<mem_type, std::string> ||
                           std::is_same_v<mem_type, std::string_view>) {
        os << "\"" << val << "\"";
      } else if constexpr (std::is_pointer_v<mem_type>) {
        os << val;
      } else if constexpr (utils::is_container_v<mem_type>) {
        os << '[';
        if (!val.empty()) {
          os << ' ';
          if (val.size() > 3) {
            os << "size: " << val.size() << " ";
          } else {
            auto itr = val.begin();
            auto itr_next = ++val.begin();
            while (itr_next != val.end()) {
              print_value<CharT, Traits, decltype(*itr), level>(os, *itr);
              os << ", ";
              ++itr;
              ++itr_next;
            }

            print_value<CharT, Traits, decltype(*itr), level>(os, *itr);
            os << ' ';
          }
        }
        os << ']';
      } else if constexpr (utils::is_map_v<mem_type>) {
        os << '{';
        if (!val.empty()) {
          os << ' ';
          if (val.size() > 3) {
            os << "size: " << val.size() << " ";
          } else {
            auto itr = val.begin();
            auto itr_next = ++val.begin();
            while (itr_next != val.end()) {
              const auto &[key, el] = *itr;
              os << key;
              os << " = ";
              print_value<CharT, Traits, decltype(el), level>(os, el);
              os << ", ";
              ++itr;
              ++itr_next;
            }

            const auto &[key, el] = *itr;
            os << key;
            os << " = ";
            print_value<CharT, Traits, decltype(el), level>(os, el);
            os << ' ';
          }
        }
        os << '}';
      } else if constexpr (std::is_function_v<mem_type>) {
        os << "(function)";
      } else {
        print_part<CharT, Traits, decltype(val), level + 1>(os, val);
        //os << '\n';
      }

      return os;
    }

    template <class CharT, class Traits, typename T, size_t level>
    std::basic_ostream<CharT, Traits>& print_part(std::basic_ostream<CharT, Traits> &os, const T &obj) {
      os << "{\n";

      reflect::for_each(
        [&](auto I) {
          using value_type = decltype(reflect::get<I>(obj));
          using mem_type = std::remove_cvref_t<value_type>;
          const auto type_name = std::is_same_v<std::string, mem_type> ? std::string_view("std::string") : (std::is_same_v<std::string_view, mem_type> ? std::string_view("std::string_view") : utils::type_name<mem_type>());
          const std::string_view name = reflect::member_name<I>(obj);
          const auto &val = reflect::get<I>(obj);
          for (size_t i = 0; i < level; ++i) { os << "  "; }
          //os << type_name << ' ';
          os << name << " = ";
          print_value<CharT, Traits, value_type, level>(os, val);
          os << ",\n";
        }, obj
      );

      for (size_t i = 0; i < level-1; ++i) { os << "  "; }
      os << '}';

      return os;
    }

    inline void print_detail() {}

    template <typename Arg, typename... Args>
    void print_detail(Arg&& arg, Args&&... args) {
      std::cout << " " << arg;
      print_detail(std::forward<Args>(args)...);
    }

    inline void print() {}

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

    template <typename T>
    constexpr size_t count_significant(T v) {
      if constexpr (std::is_enum_v<T>) { return count_significant(static_cast<int64_t>(v)); }
      else {
        size_t i = 0;
        for (; v != 0; v >>= 1, ++i) {}
        return i;
      }
    }

    // https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
    constexpr uint32_t next_power_of_2(uint32_t v) {
      v--;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      v++;

      return v;
    }

    static_assert(count_significant(1) == 1);
    static_assert(count_significant(2) == 2);
    static_assert(count_significant(3) == 2);
    static_assert(count_significant(4) == 3);
  }
    
    // poor man's struct log, needs polishing
    /*template <class chart, class traits, typename t>
    std::basic_ostream<chart, traits> &operator<<(std::basic_ostream<chart, traits> &os, const t &obj) {
      using mem_type = std::remove_cvref_t<t>;
      const auto type_name = utils::type_name<mem_type>();
      os << type_name << " ";
      return utils::print_part(os, obj);
    }*/
  }

#endif
