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
#include <functional>
#include <demiurg/resource_base.h>
#include <utils/block_allocator.h>
#include <utils/memory_pool.h>
#include <utils/type_traits.h>
// не работает со строками?
//#include <qc-hash.hpp>
// оказалось говном
//#include <utils/flat_hash_map.hpp>
#include <parallel_hashmap/phmap.h>

//template <typename K, typename T>
//using qc_hash_map = qc::hash::RawMap<K, T>;

// нужно еще предусмотреть конфиги - специальные файлы которые ПРОИЗВОДЯТ ресурсы
// они МОГУТ вернуть пачку структур которые должны превратиться в несколько ресурсов
// в чем основная проблема? нужно добавить новый ресурс и перестроить массив доступных ресурсов
// ДО того как к этому ресурсу обратятся, для конфигов можно вернуть обещалку ресурса

// разделить систему и модули, систему переназвать
// тогда в модулях можно будет удачно указать дефолтные источники ресурсов
// + разгрузить этот класс

namespace devils_engine {
namespace demiurg {
class module_system;

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

class resource_system {
public:
  struct type {
    using resource_producer = std::function<resource_interface *(utils::block_allocator &)>;

    std::string name;
    std::string ext;
    //std::array<std::string_view, 16> exts;
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

  /*struct list_entry {
    std::string path;
    std::string hash;
    std::string file_date;
  };*/

  resource_system() noexcept;
  //resource_system(std::string root) noexcept;
  ~resource_system() noexcept;

  // ext - укажем через запятую или другой знак, например png,bmp,jpg, то что первое будет считать за основной ресурс
  // и если втретится составной тип например obj,mtl, то сначала будет идти obj а потом mtl
  // здесь в аргументы можно сложить вещи которые нужны при загрузке
  // текстурки возможно все равно придется загружать с помощью хендла
  template <typename T, typename... Args>
  void register_type(std::string name, std::string ext, Args&&... args) {
    auto constructor = [args = std::make_tuple(std::forward<Args>(args)...)](
            utils::block_allocator &allocator
    ) -> resource_interface * {
      auto ptr = std::apply(&utils::block_allocator::create<T>, std::tuple_cat(std::make_tuple(std::ref(allocator)), args));
      ptr->loading_type_id = utils::type_id<T>();
      ptr->loading_type = utils::type_name<T>();
      return ptr;
    };

    auto type = types_pool.create(std::move(name), std::move(ext), sizeof(T) * 100, sizeof(T), alignof(T), std::move(constructor));
        
    types[type->name] = type;
  }

  // видимо придется немного переделать модули, чтобы они пользовались вот этим
  // модули можно вообще отсюда убрать и переместить в какой нибудь другой класс (предпочтительно)
  resource_interface *create(const std::string_view &id, const std::string_view &extension);

  //resource_system::type* find_type(const std::string_view &id, const std::string_view &extension) const;

  /*std::string_view root() const;
  void set_root(std::string root);
  std::string_view modules_list() const;
  void set_modules_list(std::string modules_list);*/

  // не указывать расширение файла!
  // поиск по отсортированному массиву поди O(logN + N)
  view<> find(const std::string_view &filter) const;
  // так работать это дело не будет, нужно отдельный контейнер делать
  template <typename T>
  size_t find(const std::string_view &filter_str, std::vector<T*> &arr) const {
    const auto v = find(filter_str);
    size_t i = 0;
    for (; i < std::min(v.size(), arr.capacity() - arr.size()); ++i) {
      if (!std::is_same_v<T, resource_interface> && v[i]->loading_type_id != utils::type_id<T>()) continue;
      auto ptr = v[i];
      arr.push_back(static_cast<T*>(ptr));
    }

    return i;
  }

  // здесь мы будем искать именно подстроку
  template <typename T>
  size_t filter(const std::string_view &filter_str, std::vector<T *> &arr) const {
    size_t counter = 0;
    for (size_t i = 0; i < resources.size() && arr.size() < arr.capacity(); ++i) {
      auto ptr = resources[i];
      if (!std::is_same_v<T, resource_interface> && ptr->loading_type_id != utils::type_id<T>()) continue;
      if (ptr->id.find(filter_str) == std::string_view::npos) continue;

      counter += 1;
      arr.push_back(static_cast<T *>(ptr));
    }

    return counter;
  }

  // при загрузке ресурсов нужно открыть все модули (ну или по крайней мере модули ресурсов)

  // наверное все удалим и заново прочитаем дерево файлов (логично)
  //void parse_file_tree();

  // конфиг сохраняем в другом месте, здесь уже сразу подгружаем созданный конфиг
  //std::vector<list_entry> load_list(const std::string_view &list_name) const;
  //void load_modules(std::vector<list_entry> ms);
  //void load_default_modules();
  void parse_resources(module_system* sys);

  //// как я ранее уже упоминал, имеет смысл вынести отсюда модули и оставить это дело только заниматься ресурсами
  //void open_modules();
  //void close_modules();

  void clear();

  size_t resources_count() const noexcept;
  size_t all_resources_count() const noexcept;
private:
  //std::string root_path;
  //std::string modules_list_name;

  utils::memory_pool<type, sizeof(type)*100> types_pool;
  phmap::flat_hash_map<std::string_view, type *> types;
  std::vector<resource_interface *> resources;
  std::vector<resource_interface *> all_resources;
  // это загруженные модули, ни в коем случае нельзя их трогать во время игры
  // список получаем из конфига
  //std::vector<std::unique_ptr<module_interface>> modules;

  resource_system::type *find_proper_type(const std::string_view &id, const std::string_view &extension) const;
  std::span<resource_interface * const> raw_find(const std::string_view &filter) const;

  // тут у нас будет по крайней мере два типа модулей: найденные модули на диске и текущие загруженные модули + структура для сохранения выбранных модулей
};

    //void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type);
}
}

#endif