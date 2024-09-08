#ifndef DEVILS_ENGINE_DEMIURG_MODULE_SYSTEM_H
#define DEVILS_ENGINE_DEMIURG_MODULE_SYSTEM_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace devils_engine {
namespace demiurg {
class resource_system;
class module_interface;

class module_system {
public:
  struct list_entry {
    std::string path;
    std::string hash;
    std::string file_date;
  };

  module_system(std::string path) noexcept;
  ~module_system() noexcept;

  std::string path() const;

  std::vector<list_entry> load_list(const std::string_view &list_name) const;
  void load_list(std::vector<list_entry> paths);
  void open_modules();
  void close_modules();
  void parse_resources(resource_system* sys);
private:
  std::string _path;
  std::vector<std::unique_ptr<module_interface>> modules;
};
}
}

#endif