#include "system.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

namespace devils_engine {
  namespace demiurg {
    system::system(std::string root) noexcept : root_path(std::move(root)) {}

    std::string_view system::root() const { return root_path; }
    void system::set_root(std::string root) { root_path = std::move(root); }

    std::span<resource_interface * const> system::find(const std::string_view &filter) const {
      const auto itr = std::lower_bound(resources.begin(), resources.end(), filter, [] (const resource_interface* const res, const std::string_view &value) {
        std::less<std::string_view> l;
        return l(res->id.substr(0, value.size()), value);
      });

      auto start = itr;
      auto end = itr;
      for (; (*itr)->id.substr(0, filter.size()) == filter && itr != resources.begin() - 1; --start) {}
      ++start;
      for (; (*itr)->id.substr(0, filter.size()) == filter && itr != resources.end(); ++end) {}
      return std::span(start, end);
    }

    void system::parse_file_tree() {
      clear();

      for (const auto &entry : fs::recursive_directory_iterator(root_path)) {
        utils::println(entry);
        // список файлов, должны распарсить путь, убрать часть root_path, преобразовать слеши в пути
        // затем посмотреть все подходящие типы для конкретного файла
        // ненаходим - игнор, находим - создаем нужный тип
      }

      std::sort(resources.begin(), resources.end(), [] (auto a, auto b) {
        std::less<std::string_view> l;
        return l(a->id, b->id);
      });
    }

    void system::clear() {
      for (auto ptr : resources) {
        ptr->~resource_interface();
      }

      // обходим все аллокаторы
    }

    void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type) {
      std::ifstream file(file_name, type);
      if (!file) utils::error("Could not load file {}", file_name);
      buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
  }
}