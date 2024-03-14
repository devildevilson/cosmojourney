#ifndef DEVILS_ENGINE_DEMIURG_SYSTEM_H
#define DEVILS_ENGINE_DEMIURG_SYSTEM_H

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>
#include <array>
#include <span>
#include <demiurg/resource_base.h>
#include <utils/block_allocator.h>

namespace devils_engine {
  namespace demiurg {
    class system {
    public:
      struct type {
        std::string name;
        std::string ext;
        std::array<std::string_view, 16> exts;
        resource_interface* type_list;
        utils::block_allocator allocator;
        // тут добавится функция создания (нужно упаковать тип в функцию)
        // разрушение - вызываем деструктор + чистим в памяти
      };

      system() noexcept = default;
      system(std::string root) noexcept;

      // расширение файла?
      template <typename T>
      void register_type(std::string name, std::string ext); // ext - укажем через запятую

      std::string_view root() const;
      void set_root(std::string root);

      // тут мы можем указать например так: sounds/ или sounds/mob1/ или sounds/mob1/rawr.ogg
      // ... стоп что с модулями? зачем нам replace если мы просто можем сохранить путь файла
      // и просто заменить с одинаковым путем файлы? логично и не придется знать откуда че у нас
      // возвращаем список найденных ресурсов
      // поиск по отсортированному массиву поди O(logN)
      std::span<resource_interface * const> find(const std::string_view &filter) const;

      // наверное все удалим и заново прочитаем дерево файлов (логично)
      void parse_file_tree();

      void clear();
    private:
      std::string root_path;

      // хранилище для всех типов + список
      std::vector<resource_interface *> resources;
    };

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type);
  }
}

// определение ресурсов расположим где то в самой игре... хотя может и нет
// тут видимо будет только определение системы

#endif