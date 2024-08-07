#ifndef DEVILS_ENGINE_DEMIURG_MODULE_INTERFACE_H
#define DEVILS_ENGINE_DEMIURG_MODULE_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace devils_engine {
namespace demiurg {
  class resource_interface;
  class system;

  // название, размер и проч полезное
  class module_interface {
  public:
    // нужен чтобы найти подходящий тип объекта
    inline module_interface(system *sys) noexcept : sys(sys) {}
    virtual ~module_interface() noexcept = default;

    // модули создаются по файлам и папкам, 1 модуль - 1 файл/папка
    // модули должны вернуть список ресурсов (???)
    // и так же из них по пути будет загрузка данных файла
    // наверное нужно добавить подготовку/очистку модуля

    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool is_openned() const = 0;
    virtual void resources_list(std::vector<resource_interface*> &arr) const = 0;
    // может быть потребуется индекс
    virtual void load_binary(const std::string_view &path, std::vector<char> &mem) const = 0;
    virtual void load_binary(const std::string_view &path, std::vector<uint8_t> &mem) const = 0;
    virtual void load_text(const std::string_view &path, std::string &mem) const = 0;

  protected:
    system *sys;
  };

  //class folder_module : public module_interface {
  //public:
  //  folder_module(system* sys, std::string root) noexcept;
  //  ~folder_module() noexcept = default;

  //  void open() override;
  //  void close() override;
  //  bool is_openned() const override;
  //  // просто пройдем все файлики в папке и добавим их в список
  //  void resources_list(std::vector<resource_interface*> &arr) const override;
  //  void load_binary(const std::string_view &path, std::vector<uint8_t> &mem) const override;
  //  void load_binary(const std::string_view &path, std::vector<char> &mem) const override;
  //  void load_text(const std::string_view &path, std::string &mem) const override;
  //private:
  //  std::string root;
  //};

  //class zip_archive_module : public module_interface {
  //public:
  //  zip_archive_module(system* sys, std::string path) noexcept;
  //  ~zip_archive_module() noexcept = default;

  //  void open() override;
  //  void close() override;
  //  bool is_openned() const override;
  //  // откроем архив и выгрузим данные из него
  //  void resources_list(std::vector<resource_interface*> &arr) const override;
  //  void load_binary(const std::string_view &path, std::vector<uint8_t> &mem) const override;
  //  void load_binary(const std::string_view &path, std::vector<char> &mem) const override;
  //  void load_text(const std::string_view &path, std::string &mem) const override;
  //private:
  //  std::string path;
  //};

  // эти два типа модуля - наверное все что имеет смысл
}
}

#endif