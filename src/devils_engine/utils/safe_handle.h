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
      ~handle_t() noexcept = default;
      handle_t(const handle_t& copy) noexcept = default;
      handle_t(handle_t&& move) noexcept = default;
      handle_t& operator=(const handle_t& copy) noexcept = default;
      handle_t& operator=(handle_t&& move) noexcept = default;
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
      safe_handle_t(const safe_handle_t& copy) noexcept = default;
      safe_handle_t(safe_handle_t&& move) noexcept = default;
      safe_handle_t & operator=(const safe_handle_t& copy) noexcept = default;
      safe_handle_t & operator=(safe_handle_t &&move) noexcept = default;

      template <typename T>
      bool is() const { return type == utils::type_id<T>(); }

      inline bool valid() const { return !is<void>(); }

      template <typename T>
      T* get() const {
        utils_assertf(is<T>(), "Handle type is not '{}' ({} != {})", utils::type_name<T>(), type, utils::type_id<T>());
        return reinterpret_cast<T*>(handle);
      }

      template <typename T>
      void set(T* ptr) {
        if (ptr == nullptr) {
          type = utils::type_id<void>();
          handle = ptr;
          return;
        }

        type = utils::type_id<T>();
        handle = ptr;
      }

      template <typename T>
      operator handle_t<T> () const { return handle_t<T>(get<T>()); }

      template <typename T>
      operator T* () const { return get<T>(); }
    };
  }
}

#endif