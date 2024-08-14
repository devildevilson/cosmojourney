#include "system.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cassert>
#include "module_interface.h"
#include "folder_module.h"
#include "utils/time.h"
#include "utils/named_serializer.h"
#include "utils/fileio.h"
#include "utils/sha256.h"
#include "utils/named_serializer.h"

namespace fs = std::filesystem;

namespace devils_engine {
  namespace demiurg {
    system::type::type(
      std::string name,
      std::string ext,
      const size_t allocator_size,
      const size_t block_size,
      const size_t allocator_align,
      resource_producer create
    ) noexcept : 
      name(std::move(name)),
      ext(std::move(ext)),
      type_list(nullptr),
      allocator(allocator_size, block_size, allocator_align),
      createf(std::move(create))
    {}

    resource_interface *system::type::create() { 
      auto ptr = createf(allocator);
      ptr->type = name;
      if (type_list == nullptr) type_list = ptr;
      else type_list->exemplary_radd(ptr);
      return ptr;
    }

    void system::type::destroy(resource_interface *ptr) {
      if (type_list == ptr) type_list = type_list->exemplary_next(type_list);
      allocator.destroy(ptr);
    }

    // по умолчанию что можно в путь положить?
    system::system() noexcept : root_path("./mods/"), modules_list_name() {}
    system::system(std::string root) noexcept : root_path(std::move(root)), modules_list_name() {
      if (root_path[root_path.size() - 1] != '/') root_path += '/';
    }

    system::~system() noexcept { 
      clear();
      for (auto & [name, ptr] : types) {
        types_pool.destroy(ptr);
      }
    }

    system::type* system::find_type(const std::string_view &id, const std::string_view &extension) const { return find_proper_type(id, extension); }

    std::string_view system::root() const { return root_path; }
    void system::set_root(std::string root) { root_path = std::move(root); }
    std::string_view system::modules_list() const { return modules_list_name; }
    void system::set_modules_list(std::string modules_list) { modules_list_name = std::move(modules_list); }

    static bool lazy_compare(const std::string_view &a, const std::string_view &b) {
      return a.substr(0, b.size()) == b;
    }

    view<> system::find(const std::string_view &filter) const {
      const auto span = raw_find(filter);
      return view<>(span.begin(), span.end());
    }

    std::span<resource_interface * const> system::raw_find(const std::string_view &filter) const {
      if (filter == "") return std::span(resources);

      const auto itr = std::lower_bound(resources.begin(), resources.end(), filter, [] (const resource_interface* const res, const std::string_view &value) {
        std::less<std::string_view> l;
        return l(res->id.substr(0, value.size()), value);
      });

      if (itr == resources.end()) return std::span<resource_interface *const>();

      auto start = resources.begin();
      auto end = itr;
      // (*start)->id.substr(0, filter.size()) != filter
      // (*end)->id.substr(0, filter.size()) == filter
      for (; start != end && !lazy_compare((*start)->id, filter); ++start) {}
      for (; end != resources.end() && lazy_compare((*end)->id, filter); ++end) {}
      return std::span(start, end);
    }

    static void parse_path(
      const std::string &path, 
      const std::string_view &root_path,
      std::string_view &module_name,
      std::string_view &file_name, 
      std::string_view &ext,
      std::string_view &id
    ) {
      std::string_view full_path = path;
      utils_assertf(full_path.find(root_path) == 0, "Path to resource must have root folder part. Current path: {}", path);
      full_path = full_path.substr(root_path.size()+1);

      const size_t first_slash = full_path.find('/');
      module_name = first_slash != std::string_view::npos ? full_path.substr(0, first_slash) : "";
      const size_t last_slash_index = full_path.rfind('/');
      file_name = last_slash_index != std::string_view::npos ? full_path.substr(last_slash_index) : full_path;
      if (file_name == "." || file_name == "..") return;

      const size_t dot_index = file_name.rfind('.');
      ext = dot_index != 0 && dot_index != std::string_view::npos ? file_name.substr(dot_index+1) : "";
      const size_t module_size = module_name == "" ? 0 : module_name.size()+1;
      const size_t ext_size = ext == "" ? 0 : ext.size()+1;
      id = full_path.substr(module_size, full_path.size() - module_size - ext_size);
    }

    static void make_forward_slash(std::string &path) {
      const char backslash = '\\';
      const char forwslash = '/';
      std::replace(path.begin(), path.end(), backslash, forwslash);
      for (auto itr = path.begin() + 1, prev = path.begin(); itr != path.end(); prev = itr, ++itr) {
        if (*itr == forwslash && *prev == forwslash) itr = path.erase(itr);
      }
    }

    // эта функция будет состоять из двух этапов:
    // 1) в рутовой папке мы ищем все файлы и папки
    // 2) файлы и папки это модули на основе которых мы построим дерево ресурсов
    // теперь нужно сделать загрузку через модули
    void system::parse_file_tree() {
      clear();

      ska::flat_hash_map<std::string_view, resource_interface *> loaded;

      for (const auto &entry : fs::recursive_directory_iterator(root_path)) {
        if (!entry.is_regular_file()) continue;
        std::string entry_path = entry.path().generic_string();
        //utils::println(entry_path);
        // generic_string уже сразу переводит строку в стандартный формат 
        // и значит скорее всего можно ожидать что и вся строка будет UTF-8 ?
//#ifdef _WIN32
//        make_forward_slash(entry_path);
//#endif

        std::string_view module_name, file_name, ext, id;
        parse_path(entry_path, root_path, module_name, file_name, ext, id);
        if (file_name == "." || file_name == "..") continue;

        auto t = find_proper_type(id, ext);

        if (t == nullptr) continue;

        auto res = t->create();
        res->set_path(entry_path, root_path);
        all_resources.push_back(res);

        // проверим загружали ли мы уже вещи
        auto itr = loaded.find(res->id);
        if (itr == loaded.end()) {
          loaded[res->id] = res;
          resources.push_back(res);
        } else {
          auto other_ptr = itr->second;
          for (; other_ptr != nullptr &&
                 other_ptr->module_name != res->module_name;
               other_ptr = other_ptr->replacement_next(itr->second)) {}

          // тут мы реплейсмент меняем и с ним уходит сапплиментари
          utils::println("res", res->module_name, res->id, "other_ptr", other_ptr != nullptr);
          if (other_ptr != nullptr) {
            // модули совпали, найдем у кого меньший индекс среди расширений
            const size_t other_place = t->ext.find(other_ptr->ext);
            const size_t res_place = t->ext.find(res->ext);
            if (res_place < other_place) {
              if (other_ptr == itr->second) {
                auto arr_itr = std::find(resources.begin(), resources.end(), other_ptr);
                (*arr_itr) = res;
              }

              auto old_repl = other_ptr->replacement_next(other_ptr);
              other_ptr->replacement_remove();
              res->replacement_radd(old_repl);

              res->supplementary_radd(other_ptr);

              // забыл поменять указатель в хеш мапе
              itr->second = res;
            } else {
              other_ptr->supplementary_radd(res);
            }
          } else {
            // новый ресурс по модулю
            // тут теперь нужно определить у кого меньший индекс 
            // примерно так же как и в случае с расширениями
            itr->second->replacement_radd(res);
          }
        }
      }

      std::sort(resources.begin(), resources.end(), [] (auto a, auto b) {
        std::less<std::string_view> l;
        return l(a->id, b->id);
      });

      for (const auto ptr : resources) {
        utils::println(ptr->module_name, ptr->id, ptr->ext);
      }
    }

    std::vector<system::list_entry> system::load_list(const std::string_view &list_name) const {
      // лист по умолчанию (чист релизная папка или архив)
      // нужно проверить... что берем по умолчанию? наверное файлик core.zip что то такое
      if (list_name.empty()) {
        const auto core_file = root_path + "core.zip";
        const auto core_folder = root_path + "core/";
        if (file_io::exists(core_file)) {
          std::vector<list_entry> e{ list_entry{ core_file, "", "" } };
          return e;
        } else if (file_io::exists(core_folder)) {
          std::vector<list_entry> e{ list_entry{ core_folder, "", "" } };
          return e;
        } 
        
        utils::error("'{}' is not exists???", core_file);
      }

      const auto path = root_path + std::string(list_name) + ".json";
      if (!file_io::exists(path)) utils::error("File '{}' not exists", path);

      const auto cont = file_io::read(path);
      std::vector<list_entry> list_entries;
      const auto ec = utils::from_json(list_entries, cont);
      if (ec) {
        utils::error("Could not parse json '{}' for struct '{}' (err code: {})", path, "std::vector<list_entry>", static_cast<size_t>(ec.ec));
      }

      return list_entries;
    }

    static std::tuple<std::string_view, std::string_view> get_name_ext(const std::string_view &path) {
      const auto ext = path.substr(path.rfind('.'));
      const auto name = path.substr(0, path.rfind('.')).substr(path.rfind('/')+1);
      return std::make_tuple(name, ext);
    }

    void system::load_modules(std::vector<list_entry> ms) {
      modules.clear();

      // примерно теже данные можно использовать для передачи информации о том
      // какие моды используются при подключении к серверу

      for (const auto &entry : ms) {
        const auto [ name, ext ] = get_name_ext(entry.path);

        fs::directory_entry e(entry.path);
        if (!e.exists()) {
          // ошибка? не, попробуем все равно загрузиться
          utils::warn("Could not find module '{}' (path: '{}')", name, entry.path);
          continue;
        }

        const auto ftime = utils::file_timestamp(e);
        const auto datetime = utils::format_localtime(ftime, "%Y-%m-%d %X");

        if (!e.is_directory()) {
          const auto cont = file_io::read<uint8_t>(entry.path);
          const auto hash = utils::SHA256::easy(cont.data(), cont.size());
          if (!entry.hash.empty() && hash != entry.hash) {
            utils::warn("Module '{}' mismatch (path: {})\nCur hash: {}\nExp hash: {}\nCur date: {}\nExp date: {}", name, entry.path, hash, entry.hash, datetime, entry.file_date);
          }
        }

        if (e.is_directory()) {
          auto mem = std::make_unique<folder_module>(this, entry.path);
          modules.push_back(std::move(mem));
        } else {
          // тут надо проверить ext, в принципе наверное тут будет только .mod и .zip
          if (ext == "mod" || ext == "zip") {
            utils::error("Zip archive is not implemented yet");
          } else {
            // (сюда попасть мы никак не должны)
            utils::warn("Module extension '{}' is not supported", ext);
            continue;
          }
        }
      }
    }

    void system::load_default_modules() {
      auto list = load_list(modules_list_name);
      load_modules(std::move(list));
    }

    void system::parse_resources() {
      clear();

      for (const auto &m : modules) { m->open(); }
      for (const auto &m : modules) { m->resources_list(all_resources); }
      for (const auto &m : modules) { m->close(); }

      ska::flat_hash_map<std::string_view, resource_interface *> loaded;
      for (const auto &res : all_resources) {
        auto t = find_proper_type(res->id, res->ext);
        if (t == nullptr) {
          utils::warn("Could not find proper type for resource '{}' extension '{}'. Skip", res->id, res->ext);
          continue;
        }

        // проверим загружали ли мы уже вещи
        auto itr = loaded.find(res->id);
        if (itr == loaded.end()) {
          loaded[res->id] = res;
          resources.push_back(res);
        } else {
          auto other_ptr = itr->second;
          for (; other_ptr != nullptr &&
                 other_ptr->module_name != res->module_name;
               other_ptr = other_ptr->replacement_next(itr->second)) {}

          // тут мы реплейсмент меняем и с ним уходит сапплиментари
          utils::println("res", res->module_name, res->id, "other_ptr", other_ptr != nullptr);
          if (other_ptr != nullptr) {
            // модули совпали, найдем у кого меньший индекс среди расширений
            const size_t other_place = t->ext.find(other_ptr->ext);
            const size_t res_place = t->ext.find(res->ext);
            if (res_place < other_place) {
              if (other_ptr == itr->second) {
                auto arr_itr = std::find(resources.begin(), resources.end(), other_ptr);
                (*arr_itr) = res;
              }

              auto old_repl = other_ptr->replacement_next(other_ptr);
              other_ptr->replacement_remove();
              res->replacement_radd(old_repl);

              res->supplementary_radd(other_ptr);

              // забыл поменять указатель в хеш мапе
              itr->second = res;
            } else {
              other_ptr->supplementary_radd(res);
            }
          } else {
            // новый ресурс по модулю
            // тут теперь нужно определить у кого меньший индекс 
            // примерно так же как и в случае с расширениями
            itr->second->replacement_radd(res);
          }
        }
      }
    }

    void system::clear() {
      for (auto ptr : all_resources) {
        const auto itr = types.find(ptr->type);
        assert(itr != types.end());
        itr->second->destroy(ptr);
      }
      resources.clear();
      all_resources.clear();
    }

    size_t system::resources_count() const noexcept { return resources.size(); }
    size_t system::all_resources_count() const noexcept { return all_resources.size(); }

    system::type * system::find_proper_type(const std::string_view &id, const std::string_view &extencion) const {
      type *t = nullptr;
      std::string_view current_full_str = id;
      while (current_full_str.size() > 0 && t == nullptr) {
        size_t end = current_full_str.size();
        while (end != std::string_view::npos && t == nullptr) {
          const auto cur_id = current_full_str.substr(0, end);
          const auto itr = types.find(cur_id);
          if (itr != types.end()) {
            auto found_t = itr->second;
            const size_t ext_index = found_t->ext.find(extencion);
            if (ext_index != std::string::npos) t = itr->second;
          }

          end = current_full_str.rfind('/', end-1);
        }

        const size_t slash_index = current_full_str.find('/');
        current_full_str = slash_index == std::string_view::npos ? "" : current_full_str.substr(slash_index+1);
      }

      return t;
    }

    void resource_interface::set_path(std::string path, const std::string_view &root) {
      this->path = std::move(path);
      std::string_view file_name;
      parse_path(this->path, root, module_name, file_name, ext, id);
    }

    void resource_interface::set(std::string path, const std::string_view &module_name, const std::string_view &id, const std::string_view &ext) {
      this->path = std::move(path);
      this->module_name = module_name;
      this->id = id;
      this->ext = ext;
    }

    resource_interface *resource_interface::replacement_next(const resource_interface *ptr) const {
      return utils::ring::list_next<list_type::replacement>(this, ptr);
    }

    resource_interface *resource_interface::supplementary_next(const resource_interface *ptr) const {
      return utils::ring::list_next<list_type::supplementary>(this, ptr);
    }

    resource_interface *resource_interface::exemplary_next(const resource_interface *ptr) const {
      return utils::ring::list_next<list_type::exemplary>(this, ptr);
    }

    void resource_interface::replacement_add(resource_interface *ptr) {
      utils::ring::list_add<list_type::replacement>(this, ptr);
    }

    void resource_interface::supplementary_add(resource_interface *ptr) {
      utils::ring::list_add<list_type::supplementary>(this, ptr);
    }

    void resource_interface::exemplary_add(resource_interface *ptr) {
      utils::ring::list_add<list_type::exemplary>(this, ptr);
    }

    void resource_interface::replacement_radd(resource_interface *ptr) {
      utils::ring::list_radd<list_type::replacement>(this, ptr);
    }

    void resource_interface::supplementary_radd(resource_interface *ptr) {
      utils::ring::list_radd<list_type::supplementary>(this, ptr);
    }

    void resource_interface::exemplary_radd(resource_interface *ptr) {
      utils::ring::list_radd<list_type::exemplary>(this, ptr);
    }

    void resource_interface::replacement_remove() {
      utils::ring::list_remove<list_type::replacement>(this);
    }

    void resource_interface::supplementary_remove() {
      utils::ring::list_remove<list_type::supplementary>(this);
    }

    void resource_interface::exemplary_remove() {
      utils::ring::list_remove<list_type::exemplary>(this);
    }

    void resource_interface::load(const utils::safe_handle_t& handle) {
      switch (_state) {
        case state::cold: load_cold(handle); break;
        case state::warm: load_warm(handle); break;
        case state::hot : break;
      }

      _state = std::max(_state + 1, 2);
    }

    void resource_interface::unload(const utils::safe_handle_t& handle) {
      switch (_state) {
        case state::cold: break;
        case state::warm: if (!flag(resource_flags::force_unload_warm)) {unload_warm(handle);} break;
        case state::hot : unload_hot(handle); break;
      }

      _state = std::min(_state - 1, 0);
    }

    void resource_interface::force_unload(const utils::safe_handle_t& handle) {
      switch (_state) {
        case state::cold: break;
        case state::warm: unload_warm(handle); break;
        case state::hot : unload_hot(handle);  break;
      }

      _state = std::min(_state - 1, 0);
    }

    enum state::values resource_interface::state() const { 
      if (_state == state::warm && flag(resource_flags::warm_and_hot_same)) return state::hot; 
      return static_cast<state::values>(_state);
    }

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type) {
      std::ifstream file(file_name, std::ios_base::openmode(type));
      if (!file) utils::error("Could not load file {}", file_name);
      buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
  }
}