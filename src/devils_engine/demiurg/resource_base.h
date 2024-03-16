#ifndef DEVILS_ENGINE_DEMIURG_RESOURCE_BASE_H
#define DEVILS_ENGINE_DEMIURG_RESOURCE_BASE_H

#include <cstddef>
#include <cstdint>
#include <utils/list.h>
#include <string>
#include <string_view>
#include <vector>
#include <bitset>
#include <boost/sml.hpp>
namespace sml = boost::sml;

#define DEMIURG_STATES_LIST \
  X(unload) \
  X(memory_load) \

#define DEMIURG_ACTIONS_LIST \
  X(unload) \
  X(load_to_memory) \

#define DEMIURG_RESOURCE_FLAGS_LIST \
  X(underlying_owner_of_raw_memory) \
  X(binary) \

namespace devils_engine {
  namespace demiurg {
    // events
    struct loading { void* ptr; };
    struct unloading { void* ptr; };

    // states
#define X(name) struct name {};
        DEMIURG_STATES_LIST
#undef X

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

    // как бы мы хотели передать ресурс в другое место?
    class resource_interface : 
      public utils::ring::list<resource_interface, 1>,
      public utils::ring::list<resource_interface, 2>
      // возможно имеет смысл составить список ресурсов по типу, но при этом у нас есть вью
      // но во вью будут вообще все записи, а тут достаточно положить только основные ресурсы 
      // (то есть главный ресурс среди соседей по названию + текущий ресурс по замене)
      //public utils::ring::list<resource_interface, utils::list_type::type>
    {
    public:
      // если мы хотим использовать ключевое слово replace где то в пути
      // то имеет смысл избавится от него в конечной строке id
      // как сделать лучше? id придется хранить отдельной строкой
      std::string path;
      std::string_view id;
      std::string_view ext;
      std::string_view module_name;
      std::string_view type;
      std::string_view loading_type;

      std::vector<char> file_memory;
      std::string_view file_text;

      size_t replacing_order;
      size_t raw_size;
      uint32_t width, height; // images
      double duration; // sound

      inline resource_interface() noexcept : replacing_order(0), raw_size(0), width(0), height(0), duration(0) {}
      virtual ~resource_interface() noexcept = default;

      void set_path(std::string path, const std::string_view &root);

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

      virtual void loading(void* userptr) = 0;
      virtual void unloading(void* userptr) = 0;

#define X(name) virtual void name(void* userptr) {}
      DEMIURG_ACTIONS_LIST
#undef X

    protected:
      // по любому будет много флагов у нас для файла, нужно битовое поле
      std::bitset<64> flags;
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

      void loading(void* userptr) override {
        process_event(demiurg::loading{userptr});
      }

      void unloading(void* userptr) override {
        process_event(demiurg::unloading{userptr});
      }

    protected:
      sml::sm<Table> sm;
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