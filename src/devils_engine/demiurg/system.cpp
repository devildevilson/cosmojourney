#include "system.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

namespace devils_engine {
  namespace demiurg {
    system::type::type(
      std::string name,
      std::string ext,
      const std::string_view &container_type,
      const size_t allocator_size,
      const size_t block_size,
      const size_t allocator_align,
      resource_producer create
    ) noexcept : 
      name(std::move(name)),
      ext(std::move(ext)),
      container_type(container_type),
      type_list(nullptr),
      allocator(allocator_size, block_size, allocator_align),
      create(std::move(create))
    {}

    system::system(std::string root) noexcept : root_path(std::move(root)) {}
    system::~system() noexcept { 
      clear();
      for (auto [name, ptr] : types) {
        types_pool.destroy(ptr);
      }
    }

    std::string_view system::root() const { return root_path; }
    void system::set_root(std::string root) { root_path = std::move(root); }

    std::span<resource_interface * const> system::find(const std::string_view &filter) const {
      const auto itr = std::lower_bound(resources.begin(), resources.end(), filter, [] (const resource_interface* const res, const std::string_view &value) {
        std::less<std::string_view> l;
        return l(res->id.substr(0, value.size()), value);
      });

      auto start = resources.begin();
      auto end = itr;
      for (; start != end && (*start)->id.substr(0, filter.size()) != filter; ++start) {}
      for (; end != resources.end() && (*end)->id.substr(0, filter.size()) == filter; ++end) {}
      return std::span(start, end);
    }

    void parse_path(
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
        if (*itr == backslash && *prev == backslash) itr = path.erase(itr);
      }
    }

    void system::parse_file_tree() {
      clear();

      for (const auto &entry : fs::recursive_directory_iterator(root_path)) {
        if (!entry.is_regular_file()) continue;
        std::string entry_path = entry.path().generic_string();
        // если позикс система то не нужно ничего делать
#ifdef _WIN32
        make_forward_slash(entry_path);
#endif

        const std::string_view full_path = entry_path;
        //const size_t root_index = entry_path.find(root_path);
        //if (root_index == 0) entry_path = entry_path.substr(root_path.size()+1); // + '/'

        std::string_view module_name, file_name, ext, id;
        parse_path(entry_path, root_path, module_name, file_name, ext, id);
        if (file_name == "." || file_name == "..") continue;

        auto t = find_proper_type(id, ext);

        if (t == nullptr) continue;

        auto res = t->create(t->allocator);
        res->set_path(entry_path, root_path);
        res->loading_type = t->container_type;
        res->type = t->name;
        // нам еще нужно найти ранее загруженные вещи
        // среди них нужно найти совпадающие id
        // и добавить их в список сопроводительных файлов
        // (типо совпадает название, но различается расширение)
        // (например x.obj и x.mtl)
        // + нужно найти одинаковые id но разные модули
        resources.push_back(res);
      }

      std::sort(resources.begin(), resources.end(), [] (auto a, auto b) {
        std::less<std::string_view> l;
        return l(a->id, b->id);
      });

      for (const auto ptr : resources) {
        utils::println(ptr->id);
      }
    }

    void system::clear() {
      for (auto ptr : resources) {
        ptr->~resource_interface();
      }
      resources.clear();
    }

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
            //const auto f = std::find(found_t->exts.begin(), found_t->exts.end(), ext);
            //if (f != found_t->exts.end()) t = *itr;
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

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type) {
      std::ifstream file(file_name, type);
      if (!file) utils::error("Could not load file {}", file_name);
      buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
  }
}