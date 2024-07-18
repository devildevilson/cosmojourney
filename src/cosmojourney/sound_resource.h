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
#define X(name) void name(sound_resource* res, const utils::safe_handle_t &handle);
  DEMIURG_ACTIONS_LIST
#undef X
  }  // namespace sound_actions_detail

  // толку с этого особенно никакого нет
  // я думал что получиться определить таблицу в cpp файле
  // но так не сработало, для чего я в принципе делал эти вещи?
  // хотел чтобы у каждого ресурса была своя стейтмашина и можно было легко понять что к чему
  // 
  struct sound_resource_table {
      auto operator()() const {
//#define X(name) const std::function<void(sound_resource* res, void* ptr)> name = &sound_actions_detail::name;
//        DEMIURG_ACTIONS_LIST
//#undef X

#define X(name) const auto l##name = [](demiurg::inj<sound_resource> i, const auto& event){ sound_actions_detail::name(i.ptr, event.handle); };
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

    void unload(const utils::safe_handle_t& handle) override;
    void load_to_memory(const utils::safe_handle_t& handle) override;

    // как то нужно собственно сам ресурс передавать из места к месту
    // но хотя это все равно будет через тот или иной прокси
    // где мы собственно преобразуем ресурс к необходимому виду
    // например при передаче ресурса в звук, мы сначала поймем что перед нами
    // преобразуем и запустим нужную функцию
    // в конфигах наверное мы будем через require получать объект с ресурсом
    // нам может пригодиться для интерфейса вся информация о ресурсе
    // в том числе также мы сделаем подгрузку скриптов
  private:
    std::unique_ptr<sound::system::resource> res;
  };
}

#endif