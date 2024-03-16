#ifndef DEVILS_ENGINE_UTILS_DICE_H
#define DEVILS_ENGINE_UTILS_DICE_H

#include <cstddef>
#include <cstdint>
#include <utils/prng.h>

namespace devils_engine {
  namespace utils {
    template <typename T>
    size_t dice(const size_t upper_bound, T &rnd) noexcept {
      using ext_t = typename T::outer;
      //const size_t value = T::value(rnd);
      //rnd = T::next(rnd);
      //const double num = utils::prng_normalize(value);
      //// практически равное распределение: к самому верхнему значению добавляется шанс 1/2^53
      //return std::min(size_t(double(upper_bound) * num), upper_bound-1);

      // возможно имеет смысл переписать вот так:
      size_t number = 0;
      do {
        const size_t value = ext_t::value(rnd);
        rnd = ext_t::next(rnd);
        const double num = utils::prng_normalize(value);
        number = size_t(double(upper_bound) * num);
      } while (number >= upper_bound); // произойдет всего один раз из 2^53

      return number;
    }

    // 2d20, 1d4
    template <typename T>
    size_t dice_accumulator(const size_t count, const size_t upper_bound, T &rnd) noexcept {
      size_t accum = 0;
      for (size_t i = 0; i < count; ++i) {
        accum += dice(upper_bound, rnd) + 1;
      }

      return accum;
    }
  }
}

#endif