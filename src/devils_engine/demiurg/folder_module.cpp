#include "folder_module.h"
#include "utils/fileio.h"
#include "utils/core.h"
#include "demiurg/system.h"
#include <filesystem>
namespace fs = std::filesystem;

namespace devils_engine {
namespace demiurg {

  static void parse_path(
      const std::string &path, 
      const std::string_view &root_path,
      std::string_view &file_name, 
      std::string_view &ext,
      std::string_view &id
    ) {
      std::string_view full_path = path;
      utils_assertf(full_path.find(root_path) == 0, "Path to resource must have root folder part. Current path: {}", path);
      full_path = full_path.substr(root_path.size()+1);

      /*const size_t first_slash = full_path.find('/');
      module_name = first_slash != std::string_view::npos ? full_path.substr(0, first_slash) : "";
      const size_t last_slash_index = full_path.rfind('/');
      file_name = last_slash_index != std::string_view::npos ? full_path.substr(last_slash_index) : full_path;*/
      file_name = full_path;
      if (file_name == "." || file_name == "..") return;

      const size_t dot_index = file_name.rfind('.');
      ext = dot_index != 0 && dot_index != std::string_view::npos ? file_name.substr(dot_index+1) : "";
      //const size_t module_size = module_name == "" ? 0 : module_name.size()+1;
      const size_t ext_size = ext == "" ? 0 : ext.size()+1;
      id = full_path.substr(0, full_path.size() - ext_size);
    }

  folder_module::folder_module(system* sys, std::string root) noexcept : module_interface(sys), _root(std::move(root)) 
  {
      const size_t index = _root.rfind('/', _root.size()-1);
      module_name = _root.substr(index);
      if (module_name.back() == '/') module_name = module_name.substr(0, module_name.size()-1);
  }

  std::string_view folder_module::name() const { return module_name; }

  void folder_module::open() {}
  void folder_module::close() {}
  bool folder_module::is_openned() const { return true; }
  void folder_module::resources_list(std::vector<resource_interface*> &arr) const {
    for (const auto &entry : fs::recursive_directory_iterator(_root)) {
      if (!entry.is_regular_file()) continue;
      std::string entry_path = entry.path().generic_string();

      // module_name мы теперь знаем и тут он нам не нужен
      std::string_view file_name, ext, id;
      parse_path(entry_path, _root, file_name, ext, id);
      if (file_name == "." || file_name == "..") continue;

      auto t = sys->find_type(id, ext);

      if (t == nullptr) {
        utils::warn("Could not find proper resource type for file '{}'", entry_path);
        continue;
      }

      auto res = t->create();
      // указание пути тоже нужно переделывать
      // наверное просто самостоятельно положить данные какие нужны
      //res->set_path(entry_path, _root);
      res->set(entry_path, module_name, id, ext);
      res->module = this;
      arr.push_back(res);
    }
  }

  void folder_module::load_binary(const std::string_view &path, std::vector<uint8_t> &mem) const {
    // полный путь? тут как будто полный путь передать неполучится, придетсся собирать строку
    mem = file_io::read<uint8_t>(_root+std::string(path));
  }
  
  void folder_module::load_binary(const std::string_view &path, std::vector<char> &mem) const {
    mem = file_io::read<char>(_root+std::string(path));
  }
  
  void folder_module::load_text(const std::string_view &path, std::string &mem) const {
    mem = file_io::read(_root+std::string(path));
  }

}
}