#ifndef DEVILS_ENGINE_SAFE_HANDLE_H
#define DEVILS_ENGINE_SAFE_HANDLE_H

#include <cstddef>
#include <cstdint>
#include <utils/core.h>
#include <utils/type_traits.h>

namespace devils_engine {
  namespace utils {
    template <typename T>
    struct handle_t {
      T* ptr;

      handle_t() noexcept : ptr(nullptr) {}
      handle_t(T* ptr) noexcept : ptr(ptr) {}
      bool valid() const { return ptr != nullptr; };
      T* get() const { return ptr; }
      T* operator->() { return ptr; }
      const T* operator->() const { return ptr; }
    };

    struct safe_handle_t {
      size_t type;
      void* handle;

      inline safe_handle_t() noexcept : type(utils::type_id<void>()), handle(nullptr) {}

      template <typename T>
      safe_handle_t(T* ptr) noexcept : type(utils::type_id<T>()), handle(ptr) {}

      ~safe_handle_t() noexcept = default;

      template <typename T>
      bool is() const { return type == utils::type_id<T>(); }

      inline bool valid() const { return !is<void>(); }

      template <typename T>
      T* get() const {
        utils_assertf(is<T>(), "Handle type is not '{}' ({} != {})", utils::type_name<T>(), type, utils::type_id<T>());
        return reinterpret_cast<T*>(handle);
      }

      template <typename T>
      operator handle_t<T> () const { return handle_t<T>(get<T>()); }

      template <typename T>
      operator T* () const { return get<T>(); }
    };
  }
}

#endif