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
#include <utils/memory_pool.h>
#include <utils/type_traits.h>
// не работает со строками?
//#include <qc-hash.hpp>
#include <utils/flat_hash_map.hpp>
#include <functional>

//template <typename K, typename T>
//using qc_hash_map = qc::hash::RawMap<K, T>;

namespace devils_engine {
  namespace demiurg {
    class system {
    public:
      struct type {
        using resource_producer = std::function<resource_interface *(utils::block_allocator &)>;

        std::string name;
        std::string ext;
        std::string_view container_type;
        std::array<std::string_view, 16> exts;
        resource_interface* type_list;
        utils::block_allocator allocator;
        resource_producer create;

        type(
          std::string name,
          std::string ext,
          const std::string_view &container_type,
          const size_t allocator_size,
          const size_t block_size,
          const size_t allocator_align,
          resource_producer create
        ) noexcept;
      };

      system() noexcept = default;
      system(std::string root) noexcept;
      ~system() noexcept;

      // ext - укажем через запятую
      template <typename T>
      void register_type(std::string name, std::string ext) {
        const auto type_name = utils::type_name<T>();
        auto type = types_pool.create(std::move(name), std::move(ext), type_name, sizeof(T) * 100, sizeof(T), alignof(T), [](utils::block_allocator &allocator) -> resource_interface * {
          return allocator.create<T>();
        });
        
        //types.insert(std::make_pair(types->name, type));
        types[type->name] = type;
      }

      std::string_view root() const;
      void set_root(std::string root);

      // не указывает расширение файла!
      // поиск по отсортированному массиву поди O(logN)
      std::span<resource_interface * const> find(const std::string_view &filter) const;

      // наверное все удалим и заново прочитаем дерево файлов (логично)
      void parse_file_tree();

      void clear();
    private:
      std::string root_path;

      // хранилище для всех типов + список
      utils::memory_pool<type, sizeof(type)*100> types_pool;
      //qc::hash::RawMap<std::string_view, type *, qc::hash::FastHash<std::string_view>> types;
      ska::flat_hash_map<std::string_view, type*> types;
      std::vector<resource_interface *> resources;

      system::type *find_proper_type(const std::string_view &id, const std::string_view &extencion) const;
    };

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type);
  }
}

// определение ресурсов расположим где то в самой игре... хотя может и нет
// тут видимо будет только определение системы

#endif