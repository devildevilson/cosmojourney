#ifndef COSMOJOURNEY_SOUND_REESOURCE_H
#define COSMOJOURNEY_SOUND_REESOURCE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>
#include <demiurg/resource_base.h>
#include <sound/system.h>
using namespace devils_engine;

namespace cosmojourney {
  class sound_resource;

  namespace sound_actions_detail {
#define X(name) void name(sound_resource* res, void* ptr);
  DEMIURG_ACTIONS_LIST
#undef X
  }  // namespace sound_actions_detail

  // толку с этого особенно никакого нет
  struct sound_resource_table {
      auto operator()() const {
//#define X(name) const std::function<void(sound_resource* res, void* ptr)> name = &sound_actions_detail::name;
//        DEMIURG_ACTIONS_LIST
//#undef X

#define X(name) const auto l##name = [](demiurg::inj<sound_resource> i, const auto& event){ sound_actions_detail::name(i.ptr, event.ptr); };
        DEMIURG_ACTIONS_LIST
#undef X

      using namespace sml;
      return make_transition_table(
        *state<demiurg::unload> + event<demiurg::loading> / lload_to_memory = state<demiurg::memory_load>,
         state<demiurg::memory_load> + event<demiurg::unloading> / lunload = state<demiurg::unload>
      );
    }
  };

  class sound_resource final : public demiurg::resource_base<sound_resource_table> {
  public:
    sound_resource() noexcept;
    ~sound_resource() noexcept = default;

    void unload(void* userptr) override;
    void load_to_memory(void* userptr) override;

  private:
    std::unique_ptr<sound::system::resource> res;
  };
}

#endif