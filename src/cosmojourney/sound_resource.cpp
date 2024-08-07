#include "sound_resource.h"
#include <utils/core.h>
#include <fstream>
#include <demiurg/system.h>
#include "demiurg/module_interface.h"

//void load_file(const std::string &file_name, std::vector<char> &buffer, const int32_t type = std::ios::binary) {
//  std::ifstream file(file_name, type);
//  if (!file) utils::error("Could not load file {}", file_name);
//  buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//}

namespace cosmojourney {
  /*auto sound_resource_table::operator()() const {
#define X(name) const auto name = [](demiurg::resource_interface* res, void* ptr) { res->name(ptr); };
    DEMIURG_ACTIONS_LIST
#undef X

    using namespace sml;
    return make_transition_table(
      *state<demiurg::unload> + event<demiurg::loading> / load_to_memory = state<demiurg::memory_load>,
       state<demiurg::memory_load> + event<demiurg::unloading> / unload = state<demiurg::unload>
    );
  }*/

  /*namespace sound_actions_detail {
#define X(name) void name(sound_resource* res, const utils::safe_handle_t &handle) { res->name(handle); }
    DEMIURG_ACTIONS_LIST
#undef X
  }*/

  sound_resource::sound_resource() noexcept //: 
  //  resource_base<sound_resource_table>(this) 
  {
    set_flag(demiurg::resource_flags::binary, true);
    set_flag(demiurg::resource_flags::warm_and_hot_same, true);
    set_flag(demiurg::resource_flags::underlying_owner_of_raw_memory, true);
    //utils::println("res ptr", this);
  }

  // тут по идее должен передаваться демиурговский стейт
  // там у нас например такие вещи как загруженные в память модули
  void sound_resource::unload_warm(const utils::safe_handle_t&) {
    // здесь че делаем? полностью выгружаем саунд рез
    res.reset(nullptr);
    raw_size = 0;
  }

  void sound_resource::unload_hot(const utils::safe_handle_t&) {}

  void sound_resource::load_cold(const utils::safe_handle_t& handle) {
    auto sound_type = sound::system::resource::type::undefined;
    for (size_t i = 0; i < static_cast<size_t>(sound::system::resource::type::undefined); ++i) {
      if (ext == sound::system::resource::type_to_string(i)) {
        sound_type = static_cast<decltype(sound_type)>(i);
        break;
      }
    }

    if (sound_type == sound::system::resource::type::undefined) {
      utils::error("Sound format '{}' is not supported, path: {}", ext, path);
    }

    // тут наверное все таки будт обращение в систему демиурга чтобы файлик получить (нет будет модуль)
    //demiurg::load_file(path, file_memory, std::ios::binary);
    module->load_binary(path, file_memory); // как то нужно подгрузить зип модуль а потом его выгрузить
    raw_size = file_memory.size();
    res = std::make_unique<sound::system::resource>(path, sound_type, std::move(file_memory));
    file_memory.clear();
    file_memory.shrink_to_fit();
    //set_flag(demiurg::resource_flags::underlying_owner_of_raw_memory, true);
    duration = res->duration();

    utils::info("Load {}", id);
  }

  void sound_resource::load_warm(const utils::safe_handle_t&) {}
}