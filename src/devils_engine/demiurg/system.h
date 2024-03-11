#ifndef DEVILS_ENGINE_DEMIURG_SYSTEM_H
#define DEVILS_ENGINE_DEMIURG_SYSTEM_H

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>
#include <boost/sml.hpp>
#include <utils/list.h>
namespace sml = boost::sml;

namespace devils_engine {
  namespace demiurg {
    class system {
    public:
      struct type {
        std::string name;
        // экстеншены
        resource_interface* type_list;
      };

      system() noexcept = default;
      system(std::string root) noexcept;

      template <typename T>
      void register_type(std::string type); // расширение файла?

    };

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
      std::string_view module;
      std::string_view type;
      std::string_view loading_type;

      std::vector<char> file_memory;
      std::string_view file_text;

      size_t replacing_order;

      // по любому будет много флагов у нас для файла, нужно битовое поле

      virtual ~resource_interface() = default;

      // декоратор?
      virtual void action1() = 0;
      virtual void action2() = 0;
      virtual void action3() = 0;
    };

    template <typename Table>
    class resource_base : public resource_interface {
    public:
      inline resource_base() noexcept : sm{static_cast<resource_interface*>(this)} {}

      template <typename T>
      bool is(T&& arg) const {
        return sm.is(std::forward<T>(arg));
      }
      
      // несколько аргументов?
      template <typename T>
      void process_event(T&& arg) {
        sm.process_event(std::forward<T>(arg));
      }

    protected:
      sml::sm<Table> sm;
    };

    struct state1 {};
    struct state2 {};
    struct state3 {};
    struct event1 {};
    struct event2 {};
    struct event3 {};
    struct timeout {};
    struct initial {};

    struct resource_table1 {
      inline auto operator()() const noexcept {
        const auto action1 = [](resource_interface* res) { res->action1(); };
        const auto action2 = [](resource_interface* res) { res->action2(); };
        const auto action3 = [](resource_interface* res) { res->action3(); };

        using namespace sml;
        return make_transition_table(
          *state<initial> + event<event1> / action1 = state<state1>,
           state<state1>  + event<event2> / action2 = state<state2>,
           state<state2>  + event<event3> / action3 = state<state3>,
           state<state3>  + event<timeout> = X
        );
      }
    };

    // пример, типы ресурсов понятное дело разнесем в другие места
    // необязательно все действия определять
    class resource_child1 : public resource_base<resource_table1> {
    public:
      void action1() override {
        printf("child1 action1\n");
      }

      void action2() override {
        printf("child1 action2\n");
      }

      void action3() override {
        printf("child1 action3\n");
      }
    };
  }
}

// определение ресурсов расположим где то в самой игре... хотя может и нет
// тут видимо будет только определение системы

#endif