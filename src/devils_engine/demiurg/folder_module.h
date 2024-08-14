#ifndef DEVILS_ENGINE_DEMIURG_FOLDER_MODULE_H
#define DEVILS_ENGINE_DEMIURG_FOLDER_MODULE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "demiurg/module_interface.h"

namespace devils_engine {
namespace demiurg {
  class folder_module : public module_interface {
  public:
    folder_module(system* sys, std::string root) noexcept;
    ~folder_module() noexcept = default;

    std::string_view name() const;

    void open() override;
    void close() override;
    bool is_openned() const override;
    // просто пройдем все файлики в папке и добавим их в список
    void resources_list(std::vector<resource_interface*> &arr) const override;
    void load_binary(const std::string_view &path, std::vector<uint8_t> &mem) const override;
    void load_binary(const std::string_view &path, std::vector<char> &mem) const override;
    void load_text(const std::string_view &path, std::string &mem) const override;
    std::vector<uint8_t> load_binary(const std::string_view &path) const override;
    std::string load_text(const std::string_view &path) const override;
  private:
    std::string_view module_name;
  };
}
}

#endif