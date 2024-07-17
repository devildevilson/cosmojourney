#ifndef DEVILS_ENGINE_UTILS_FAST_SERIALIZER_H
#define DEVILS_ENGINE_UTILS_FAST_SERIALIZER_H

#include <string>
#include <stdexcept>
#include <iostream>
#include <reflect>
#include <vector>
#include <array>
#include <format>
#include <glaze/glaze.hpp>
#include "utils/core.h"
#include "utils/type_traits.h"
#include "utils/string.h"

// для json лучше использовать glaze
// для lua имеет смысл написать десериализатор (позже)

namespace devils_engine {
namespace utils {

//template <typename T>
//bool to_json(const T &x, std::string &c);
//
//template <typename T>
//bool to_json_value(const T &val, std::string &c) {
//  using mem_type = std::remove_cvref_t<T>;
//  if constexpr (std::is_same_v<mem_type, bool>) {
//    c.append(val ? "true" : "false");
//    return true;
//  } else if constexpr (std::is_arithmetic_v<mem_type>) {
//    c.append(std::format("{}", val));
//    return true;
//  } else if constexpr (std::is_same_v<mem_type, const char *> ||
//                       std::is_same_v<mem_type, std::string> ||
//                       std::is_same_v<mem_type, std::string_view>) {
//    c.push_back('"');
//    c.append(val);
//    c.push_back('"');
//    return true;
//  } else if constexpr (is_container_v<mem_type>) {
//    c.push_back('[');
//    if (!val.empty()) {
//      for (const auto &el : val) {
//        to_json_value(el, c);
//        c.push_back(',');
//      }
//      c.pop_back(); // лишняя запятая по идее
//    }
//    c.push_back(']');
//    return true;
//  } else if constexpr (is_map_v<mem_type>) {
//    c.push_back('{');
//    if (!val.empty()) {
//      for (const auto & [ key, el ] : val) {
//        to_json_value(key, c);
//        c.push_back(':');
//        to_json_value(el, c);
//        c.push_back(',');
//      }
//      c.pop_back();  // лишняя запятая по идее
//    }
//    c.push_back('}');
//    return true;
//  } else if constexpr (std::is_pointer_v<mem_type>) {
//    return false;
//  } else {
//    to_json(val, c);
//    return true;
//  }
//}
//
//template <typename T> //, typename Md = describe_members<T, mod_any_access>
//bool to_json(const T &x, std::string &c) {
//  c.push_back('{');
//
//  reflect::for_each([&](auto I) {
//    using mem_type = std::remove_cvref_t<decltype(reflect::get<I>(x))>;
//    const std::string_view name = reflect::member_name<I>(x);
//    const auto &val = reflect::get<I>(x);
//    to_json_value(name, c);
//    c.push_back(':');
//    const bool ret = to_json_value(val, c);
//    if (!ret) utils::error("Could not serialize value of type '{}' ({})", utils::type_name<mem_type>(), name);
//    c.push_back(',');
//    //utils::println(utils::type_name<mem_type>(), reflect::member_name<I>(x), reflect::get<I>(x));
//  }, x);
//
//  if (c.back() != '{') c.pop_back();
//  c.push_back('}');
//  return true;
//}

template <typename T>
void to_json(const T &x, std::string &c) {
  glz::write_json(x, c);
}

template <typename T>
void from_json(T &x, const std::string_view &c) {
  const auto ec = glz::read_json(x, c);
  if (ec) {
    utils::error("Could not parse json for struct '{}' ({})", utils::type_name<T>(), c);
  }
}


template <typename T>
bool to_lua_impl(const T &x, std::string &c);

template <typename T>
bool to_lua_value(const T &val, std::string &c) {
  using mem_type = std::remove_cvref_t<T>;
  if constexpr (std::is_same_v<mem_type, bool>) {
    c.append(val ? "true" : "false");
    return true;
  } else if constexpr (std::is_arithmetic_v<mem_type>) {
    c.append(std::format("{}", val));
    return true;
  } else if constexpr (std::is_same_v<mem_type, const char *> ||
                       std::is_same_v<mem_type, std::string> ||
                       std::is_same_v<mem_type, std::string_view>) {
    c.push_back('"');
    c.append(val);
    c.push_back('"');
    return true;
  } else if constexpr (is_container_v<mem_type>) {
    c.push_back('{');
    if (!val.empty()) {
      for (const auto &el : val) {
        to_lua_value(el, c);
        c.push_back(',');
      }
      c.pop_back();  // лишняя запятая по идее
    }
    c.push_back('}');
    return true;
  } else if constexpr (is_map_v<mem_type>) {
    c.push_back('{');
    if (!val.empty()) {
      for (const auto &[key, el] : val) {
        c.append(key);
        c.push_back('=');
        to_lua_value(el, c);
        c.push_back(',');
      }
      c.pop_back();  // лишняя запятая по идее
    }
    c.push_back('}');
    return true;
  } else if constexpr (std::is_pointer_v<mem_type>) {
    return false;
  } else {
    to_lua_impl(val, c);
    return true;
  }
}

template <typename T>
bool to_lua_impl(const T &x, std::string &c) {
  c.push_back('{');

  reflect::for_each([&](auto I) {
    using mem_type = std::remove_cvref_t<decltype(reflect::get<I>(x))>;
    const std::string_view name = reflect::member_name<I>(x);
    const auto &val = reflect::get<I>(x);
    c.append(name);
    c.push_back('=');
    const bool ret = to_lua_value(val, c);
    if (!ret) utils::error("Could not serialize value of type '{}' ({})", utils::type_name<mem_type>(), name);
    c.push_back(',');
  }, x);

  if (c.back() != '{') c.pop_back();
  c.push_back('}');
  return true;
}

template <typename T>
bool to_lua(const T &x, std::string &c) {
  to_lua_impl(x, c);
  c = "return " + c;
  return true;
}

template <typename T>
bool from_json(const std::string_view &str, T &x) {
  auto ins_str = utils::string::trim(utils::string::inside(str, "{", "}"));
  while (ins_str != "") {
    
  }
}
}
}

#endif