#ifndef DEVILS_ENGINE_UTILS_STRING_H
#define DEVILS_ENGINE_UTILS_STRING_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <cctype>
#include <algorithm>

namespace devils_engine {
  namespace utils {
    namespace string {
      // должен возвращать размер массива + SIZE_MAX если больше max_arr, 
      // если SIZE_MAX то последним аргументом должна быть оставшаяся строка
      constexpr size_t split(const std::string_view &input, const std::string_view &token, std::string_view *arr, const size_t max_arr) {
        size_t count = 0;
        size_t prev_pos = 0;
        size_t current_pos = 0;
        do {
          current_pos = input.find(token, prev_pos);

          const size_t substr_count = count + 1 == max_arr ? std::string_view::npos : current_pos - prev_pos;
          arr[count] = input.substr(prev_pos, substr_count);
          count += 1;
          prev_pos = current_pos + token.size();
        } while (current_pos < input.size() && count < max_arr);

        count = count == max_arr && current_pos != std::string_view::npos ? SIZE_MAX : count;

        return count;
      }

      constexpr std::string_view inside(const std::string_view &input, const std::string_view &right, const std::string_view &left) {
        const size_t start = input.find(right);
        const size_t end = input.rfind(left);
        if (start >= end) return "";
        if (end == std::string_view::npos) return "";

        return input.substr(start + right.size(), end - (start + right.size()));
      }

      constexpr bool is_whitespace(char c) {
        // Include your whitespaces here. The example contains the characters
        // documented by https://en.cppreference.com/w/cpp/string/wide/iswspace
        constexpr char matches[] = { ' ', '\n', '\r', '\f', '\v', '\t' };
        return std::any_of(std::begin(matches), std::end(matches), [c](char c0) { return c == c0; });
      }

      constexpr std::string_view trim(const std::string_view &input) {
        size_t right = 0; 
        size_t left = input.size() - 1;

        bool right_isspace = is_whitespace(input[right]);
        bool left_isspace = is_whitespace(input[left]);

        while (right <= left && (right_isspace || left_isspace)) {
          right += size_t(right_isspace);
          left -= size_t(left_isspace);
          right_isspace = is_whitespace(input[right]);
          left_isspace = is_whitespace(input[left]);
        }

        if (right >= left) return "";
        // это индексы поэтому +1
        return input.substr(right, left - right + 1);
      }
    }
  }
}

#endif