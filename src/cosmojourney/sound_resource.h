#ifndef COSMOJOURNEY_SOUND_REESOURCE_H
#define COSMOJOURNEY_SOUND_REESOURCE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <demiurg/resource_base.h>
#include <sound/system.h>
using namespace devils_engine;

namespace cosmojourney {
  struct sound_resource_table {
      auto operator()() const {
#define X(name) const auto name = [](demiurg::resource_interface* res, void* ptr) { res->name(ptr); };
      DEMIURG_ACTIONS_LIST
#undef X

      using namespace sml;
      return make_transition_table(
        *state<demiurg::unload> + event<demiurg::loading> / load_to_memory = state<demiurg::memory_load>,
         state<demiurg::memory_load> + event<demiurg::unloading> / unload = state<demiurg::unload>
      );
    }
  };

  class sound_resource : demiurg::resource_base<sound_resource_table> {
  public:
    sound_resource() noexcept = default;
    ~sound_resource() noexcept = default;

    void unload(void* userptr) override;
    void load_to_memory(void* userptr) override;

  private:
    std::unique_ptr<sound::system::resource> res;
  };
}

#endif