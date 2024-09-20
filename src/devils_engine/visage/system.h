#ifndef DEVILS_ENGINE_VISAGE_SYSTEM_H
#define DEVILS_ENGINE_VISAGE_SYSTEM_H

#include <cstddef>
#include <cstdint>
#include <chrono>

namespace devils_engine {
namespace visage {

// где то тут наверное будет храниться отдельный луа стейт чисто под интерфейс
// + энвайронмент 
// + nk_context
// и тут мы зададим собственно все биндинги для интерфейса
// и каждый кадр будем обновлять функцией update
// + будет шаг в котором нужно будет скинуть все данные по контексту в буфер

// для каких то других систем которые используют луа будет другой хук и другое время 
// больше всего меня волнует генератор, как бы не попасть в просак с ним

class system {
public:
  static constexpr int hook_after_instructions_count = 100000;
  static constexpr size_t seconds10 = 10ull * 1000ull * 1000ull;
  using clock_t = std::chrono::high_resolution_clock;
  static clock_t::time_point start_tp;
  static size_t instruction_counter;

  void update(const size_t time);
};
}
}

#endif