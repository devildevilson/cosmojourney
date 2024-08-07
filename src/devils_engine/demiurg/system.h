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
    template <typename T = resource_interface *const>
    class view_iterator : public std::span<resource_interface *const>::iterator {
    public:
      using super = typename std::span<resource_interface *const>::iterator;

      template <typename... Args>
      view_iterator(Args &&...args) : super(std::forward<Args>(args)...) {}

      T &operator*() noexcept { return static_cast<T>(super::operator*()); }
      const T &operator*() const noexcept { return static_cast<T>(super::operator*()); }
    };

    template <typename T = resource_interface *const>
    class view : public std::span<resource_interface *const> {
    public:
      using super = std::span<resource_interface *const>;
      using iterator = view_iterator<T>;
      using reverse_iterator = std::reverse_iterator<iterator>;

      template<typename... Args>
      view(Args &&...args) : super(std::forward<Args>(args)...) {}

      T & operator[](const size_t index) { return static_cast<T>(super::operator[](index)); }
      const T & operator[](const size_t index) const { return static_cast<T>(super::operator[](index)); }

      iterator begin() const noexcept { return iterator(super::begin()); }
      iterator end() const noexcept { return iterator(super::end()); }
      reverse_iterator rbegin() const noexcept { return reverse_iterator(super::rbegin()); }
      reverse_iterator rend() const noexcept { return reverse_iterator(super::rend()); }
    };

    struct module_data {
      std::string path;
      std::string_view name;
      std::string hash; // как считать хеш для папок?
      size_t workshop_id;
      // метка времени создания? 
      resource_interface* thumbnail;
      resource_interface* description; // да его поди сразу загрузить нужно
    };

    class system {
    public:
      struct type {
        using resource_producer = std::function<resource_interface *(utils::block_allocator &)>;

        std::string name;
        std::string ext;
        std::array<std::string_view, 16> exts;
        resource_interface* type_list;
        utils::block_allocator allocator;
        resource_producer createf;

        type(
          std::string name,
          std::string ext,
          const size_t allocator_size,
          const size_t block_size,
          const size_t allocator_align,
          resource_producer create
        ) noexcept;

        resource_interface *create();
        void destroy(resource_interface * ptr);
      };

      system() noexcept;
      system(std::string root) noexcept;
      ~system() noexcept;

      // ext - укажем через запятую
      template <typename T>
      void register_type(std::string name, std::string ext) {
        auto type = types_pool.create(std::move(name), std::move(ext), sizeof(T) * 100, sizeof(T), alignof(T), [](utils::block_allocator &allocator) -> resource_interface * {
          auto ptr = allocator.create<T>();
          ptr->loading_type_id = utils::type_id<T>();
          ptr->loading_type = utils::type_name<T>();
          return ptr;
        });
        
        //types.insert(std::make_pair(types->name, type));
        types[type->name] = type;
      }

      system::type* find_type(const std::string_view &id, const std::string_view &extension) const;

      std::string_view root() const;
      void set_root(std::string root);

      // не указывать расширение файла!
      // поиск по отсортированному массиву поди O(logN + N)
      view<> find(const std::string_view &filter) const;
      // так работать это дело не будет, нужно отдельный контейнер делать
      template <typename T>
      size_t find(const std::string_view &filter, std::vector<T* const> &arr) const {
        const auto v = find(filter);
        for (size_t i = 0; i < std::min(v.size(), arr.capacity() - arr.size()); ++i) {
          if (v[i]->loading_type_id != utils::type_id<T>()) continue;
          arr.push_back(static_cast<T *const>(v[i]));
        }

        return v.size();
      }

      // наверное все удалим и заново прочитаем дерево файлов (логично)
      void parse_file_tree();

      // при входе в саму игру грузим модули с конфига
      void load_modules();

      // можно ли вызвать несколько раз? можно, но при этом несколько раз создадим thumbnail, хотя его поди можно найти где нибудь
      // прочитаем файлики на диске, и видимо придется предварительно подгрузить архивы и выгрузить оттуда картинки и описание
      std::vector<module_data> get_modules();

      // во первых сохраним в конфиг последнюю настройку модулей
      // во вторых было бы неплохо настройку сохранить под разными профилями
      void save_list(std::string name, std::vector<module_data> data);
      std::vector<module_data> load_list(std::string name);
      std::vector<std::string> available_lists();

      void clear();

      size_t resources_count() const noexcept;
      size_t all_resources_count() const noexcept;
    private:
      std::string root_path;
      std::string current_modules_list;

      utils::memory_pool<type, sizeof(type)*100> types_pool;
      //qc::hash::RawMap<std::string_view, type *, qc::hash::FastHash<std::string_view>> types;
      ska::flat_hash_map<std::string_view, type*> types;
      std::vector<resource_interface *> resources;
      std::vector<resource_interface *> all_resources;
      // это загруженные модули, ни в коем случае нельзя их трогать во время игры
      // список получаем из конфига
      std::vector<std::unique_ptr<module_interface>> modules;

      // здесь еще будет отдельный список модулей + список списков модулей
      // наверное список получим и отдадим его вовне, что должно быть в списке?
      // полный путь + время создания + название + (возможно) id воркшопа + хеш + ресурс картинки + ресурс с описанием
      // получаем на вход список модулей в определенном порядке, по ним создаем конфиг на диск
      // затем при загрузке считываем конфиг

      system::type *find_proper_type(const std::string_view &id, const std::string_view &extension) const;
      std::span<resource_interface * const> raw_find(const std::string_view &filter) const;

      // тут у нас будет по крайней мере два типа модулей: найденные модули на диске и текущие загруженные модули + структура для сохранения выбранных модулей
    };

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type);
  }
}

// определение ресурсов расположим где то в самой игре... хотя может и нет
// тут видимо будет только определение системы

#endif