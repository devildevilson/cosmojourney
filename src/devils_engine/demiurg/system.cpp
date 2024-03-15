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
        // список файлов, должны распарсить путь, убрать часть root_path, преобразовать слеши в пути
        // затем посмотреть все подходящие типы для конкретного файла
        // ненаходим - игнор, находим - создаем нужный тип
        if (!entry.is_regular_file()) continue;
        std::string entry_path = entry.path().generic_string();
        // если позикс система то не нужно ничего делать
#ifdef _WIN32
        make_forward_slash(entry_path);
#endif

        // дальше нужно получить модуль + id + ext + тип
        // bebra  /  sound/res/mob . ogg
        // модуль         id         ext
        // тип посмотрим вот так: сначала sound/res потом sound
        const size_t root_index = entry_path.find(root_path);
        if (root_index == 0) entry_path = entry_path.substr(root_path.size()+1); // + '/'

        // теперь entry_path - это полный путь ресурса, строка до первого слеша - это модуль
        // или если это архив то это модуль
        const std::string_view full_path = entry_path;
        const size_t first_slash = full_path.find('/');
        const auto module_name = first_slash != std::string_view::npos ? full_path.substr(0, first_slash) : "";
        const size_t last_slash_index = full_path.rfind('/');
        const auto file_name = last_slash_index != std::string_view::npos ? full_path.substr(last_slash_index) : full_path;
        if (file_name == "." || file_name == "..") continue;

        const size_t dot_index = file_name.rfind('.');
        const auto ext = dot_index != 0 && dot_index != std::string_view::npos ? file_name.substr(dot_index+1) : "";
        const size_t module_size = module_name == "" ? 0 : module_name.size()+1;
        const size_t ext_size = ext == "" ? 0 : ext.size()+1;
        const auto id = full_path.substr(module_size, full_path.size() - module_size - ext_size);

        // так теперь у нас есть id по которому я могу глянуть его обработчик
        // обработчик мы смотрим как? 
        // обойдем все слешы последовательно сокращая строку 
        // затем начем со строкой +1 вложенностью
        // то есть
        auto itr = 123;
        std::string_view current_full_str = id;
        size_t start = 0;
        while (current_full_str.size() > 0) {
          size_t end = current_full_str.size();
          while (end != std::string_view::npos) {
            const auto cur_id = current_full_str.substr(0, end);
            //utils::println("end", cur_id, end);
            // чекаем id

            end = current_full_str.rfind('/', end-1);
          }

          const size_t slash_index = current_full_str.find('/');
          current_full_str = slash_index == std::string_view::npos ? "" : current_full_str.substr(slash_index+1);
          //utils::println("cur", current_full_str);
        }

        utils::println(entry_path);
        //utils::println(module_name, id, ext);
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