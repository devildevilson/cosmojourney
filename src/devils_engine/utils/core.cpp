#include "core.h"

namespace devils_engine {
  namespace utils {
    std::string_view make_sane_file_name(const std::string_view &str) {
      const size_t slash1_index = str.rfind("/");
      if (slash1_index == std::string_view::npos) return str;

      const auto str2 = str.substr(0, slash1_index);
      const size_t slash2_index = str2.rfind("/");
      return slash2_index == std::string_view::npos ? str.substr(slash1_index+1) : str.substr(slash2_index+1);
    }

    void assert_failed_detail(
      const std::string_view &cond_str,
      const std::string_view &file_name,
      const std::string_view &func_name,
      const size_t line
    ) {
      spdlog::error("{}:{}: {}: Assertion `{}` failed", make_sane_file_name(file_name), line, func_name, cond_str);
      throw std::runtime_error("Assertion failed");
    }

    void assert_failed_detail(
      const std::string_view &cond_str,
      const std::string_view &file_name,
      const std::string_view &func_name,
      const size_t line,
      const std::string_view &comm
    ) {
      spdlog::error("{}:{}: {}: Assertion `{}` failed", make_sane_file_name(file_name), line, func_name, cond_str);
      spdlog::info(comm);
      throw std::runtime_error("Assertion failed");
    }

    time_log::time_log(std::string str) noexcept : str(std::move(str)), tp(std::chrono::steady_clock::now()) {}
    time_log::~time_log() noexcept {
      const auto dur = std::chrono::steady_clock::now() - tp;
      const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
      //println(str, "took", mcs, "mcs");
      info("{} took {} mcs ({:.3} seconds)", str, mcs, double(mcs)/1000000.0);
    }

    tracer::tracer(std::source_location loc) noexcept : l(std::move(loc)) {
      spdlog::log(spdlog::level::trace, "in  {}:{} `{}`", make_sane_file_name(l.file_name()), l.line(), l.function_name());
    }

    tracer::~tracer() noexcept {
      spdlog::log(spdlog::level::trace, "out {}:{} `{}`", make_sane_file_name(l.file_name()), l.line(), l.function_name());
    }
  }
}
