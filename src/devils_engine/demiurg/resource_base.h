#ifndef DEVILS_ENGINE_DEMIURG_RESOURCE_BASE_H
#define DEVILS_ENGINE_DEMIURG_RESOURCE_BASE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <bitset>
#include <utils/list.h>
#include <utils/safe_handle.h>
#include <boost/sml.hpp>
namespace sml = boost::sml;

#define DEMIURG_STATES_LIST2 \
  X(unload) \
  X(memory_load) \

#define DEMIURG_STATES_LIST \
  X(cold)                  \
  X(warm)                  \
  X(hot)                   \

#define DEMIURG_ACTIONS_LIST2 \
  X(unload) \
  X(load_to_memory) \

// по сути действия будет только 2, загрузить/выгрузить
// загрузка переводит на стейт ниже в DEMIURG_STATES_LIST2, выгрузка на стейт выше
// при этом должны ли ресурсы следить за свои состоянием?
// или состояние можно сделать тут?
// в основном что мне не нравится - я могу встретить неподготовленную картинку
// что делать? вылетать по ошибке? не хотелось бы, но при этом в продакшене иного выбора как будто нет
#define DEMIURG_ACTIONS_LIST  \
  X(load_cold)                \
  X(load_warm)                \
  X(unload_warm)              \
  X(unload_hot)               \

#define DEMIURG_RESOURCE_FLAGS_LIST \
  X(underlying_owner_of_raw_memory) \
  X(binary)                         \
  X(warm_and_hot_same)              \
  X(force_unload_warm)              \

namespace devils_engine {
  namespace demiurg {
    class module_interface;

    // events
    struct loading { utils::safe_handle_t handle; };
    struct unloading { utils::safe_handle_t handle; };

    // states
//#define X(name) struct name {};
//        DEMIURG_STATES_LIST
//#undef X

    namespace state {
      enum values {
#define X(name) name,
        DEMIURG_STATES_LIST
#undef X

        count
      };
    }

    namespace resource_flags {
      enum values {
#define X(name) name,
        DEMIURG_RESOURCE_FLAGS_LIST
#undef X

        count
      };
    }

    namespace list_type {
      enum values {
        replacement,
        supplementary,
        exemplary,
        count
      };
    }

    // как бы мы хотели передать ресурс в другое место?
    class resource_interface : 
      public utils::ring::list<resource_interface, list_type::replacement>,
      public utils::ring::list<resource_interface, list_type::supplementary>,
      public utils::ring::list<resource_interface, list_type::exemplary>
    {
    public:
      std::string path;
      std::string_view id;
      std::string_view ext;
      std::string_view module_name;
      std::string_view type;
      std::string_view loading_type;
      size_t loading_type_id;

      const module_interface* module;

      std::vector<char> file_memory;
      std::string_view file_text;

      size_t replacing_order;
      size_t raw_size;

      // userdata
      // наверное еще дополнительно потребуется тип
      uint32_t width, height; // images
      double duration; // sound

      inline resource_interface() noexcept : 
        loading_type_id(0), 
        replacing_order(0),
        raw_size(0),
        width(0),
        height(0),
        duration(0) 
      {}
      virtual ~resource_interface() noexcept = default;

      void set_path(std::string path, const std::string_view &root);
      void set(std::string path, const std::string_view &module_name, const std::string_view &id, const std::string_view &ext);

      resource_interface* replacement_next(const resource_interface* ptr) const;
      resource_interface* supplementary_next(const resource_interface* ptr) const;
      resource_interface* exemplary_next(const resource_interface* ptr) const;

      void replacement_add(resource_interface* ptr);
      void supplementary_add(resource_interface* ptr);
      void exemplary_add(resource_interface* ptr);

      void replacement_radd(resource_interface* ptr);
      void supplementary_radd(resource_interface* ptr);
      void exemplary_radd(resource_interface* ptr);

      void replacement_remove();
      void supplementary_remove();
      void exemplary_remove();

      template <typename T>
      bool flag(const T &index) const {
        return flags.test(static_cast<size_t>(index));
      }

      template <typename T>
      void set_flag(const T &index, const bool val) {
        flags.set(static_cast<size_t>(index), val);
      }

      // декоратор?
      // нет тут по итогу нужно совсем другое
      // достаточно определить 2 функции 
      // событие последовательной загрузки и 
      // событие последовательной выгрузки

      // и достаточно определить их в resource_base
      // и все, все конкретные функции можно уже определить в дочернем классе

      //virtual void loading(const utils::safe_handle_t &handle) = 0;
      //virtual void unloading(const utils::safe_handle_t &handle) = 0;

#define X(name) virtual void name(const utils::safe_handle_t &handle) = 0;
      DEMIURG_ACTIONS_LIST
#undef X

      void load(const utils::safe_handle_t& handle) {
        switch (_state) {
          case state::cold: load_cold(handle); break;
          case state::warm: load_warm(handle); break;
          case state::hot : break;
        }

        _state = std::max(_state + 1, 2);
      }

      void unload(const utils::safe_handle_t& handle) {
        switch (_state) {
          case state::cold: break;
          case state::warm: if (!flag(resource_flags::force_unload_warm)) {unload_warm(handle);} break;
          case state::hot : unload_hot(handle); break;
        }

        _state = std::min(_state - 1, 0);
      }

      void force_unload(const utils::safe_handle_t& handle) {
        switch (_state) {
          case state::cold: break;
          case state::warm: unload_warm(handle); break;
          case state::hot : unload_hot(handle);  break;
        }

        _state = std::min(_state - 1, 0);
      }

      enum state::values state() const { 
        if (_state == state::warm && flag(resource_flags::warm_and_hot_same)) return state::hot; 
        return static_cast<state::values>(_state);
      }
    protected:
      // по любому будет много флагов у нас для файла, нужно битовое поле
      std::bitset<64> flags;
      int32_t _state;
    };

    template <typename T>
    struct inj { T* ptr; };

    template <typename Table>
    class resource_base : public resource_interface {
    public:
      inline resource_base() noexcept : sm{inj{static_cast<resource_interface*>(this)}} {}

      template <typename T>
      resource_base(T* ptr) noexcept : sm{inj{ptr}} {}

      template <typename T>
      bool is(T&& arg) const {
        return sm.is(std::forward<T>(arg));
      }
  
      // несколько аргументов?
      template <typename... Args>
      void process_event(Args&&... arg) {
        sm.process_event(std::forward<Args>(arg)...);
      }

      void loading(const utils::safe_handle_t &handle) override {
        process_event(demiurg::loading{handle});
      }

      void unloading(const utils::safe_handle_t& handle) override {
        process_event(demiurg::unloading{handle});
      }

      // скорее всего это все что нужно
      // handle нужно будет менять в зависимости от стейта
      void load2(const utils::safe_handle_t& handle) {
        switch (state) {
          case 0: load_cold(handle); break;
          case 1: load_warm(handle); break;
          case 3: break;
        }

        state = std::max(state + 1, 2);
      }

      void unload2(const utils::safe_handle_t& handle) {
        switch (state) {
          case 0: break;
          case 1: unload_warm(handle); break;
          case 2: unload_hot(handle);  break;
        }

        state = std::min(state - 1, 0);
      }

    protected:
      sml::sm<Table> sm; // ненужен
      int32_t state;
    };

    void parse_path(
      const std::string& path, 
      std::string_view& module_name,
      std::string_view& file_name,
      std::string_view& ext,
      std::string_view& id
    );
  }
}

#endif