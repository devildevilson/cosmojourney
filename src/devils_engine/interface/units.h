#ifndef DEVILS_ENGINE_INTERFACE_UNITS_H
#define DEVILS_ENGINE_INTERFACE_UNITS_H

#define DE_INTEFACE_UNITS_LIST \
  X(absolute) \
  X(density_independent) \
  X(scaled_pixels) \
  X(relative) \

#define SUI_INLINE inline

namespace devils_engine {
  namespace sui {
    struct context;

#define X(name) \
    struct name { \
      float value; \
      SUI_INLINE constexpr name() noexcept : value(0.0) {} \
      SUI_INLINE constexpr name(const float value) noexcept : value(value) {} \
      SUI_INLINE constexpr ~name() noexcept = default; \
      SUI_INLINE constexpr name(const name &copy) noexcept = default; \
      SUI_INLINE constexpr name(name &&move) noexcept = default; \
      SUI_INLINE constexpr name & operator=(const name &copy) noexcept = default; \
      SUI_INLINE constexpr name & operator=(name &&move) noexcept = default; \
      SUI_INLINE constexpr name & operator+=(const name &v) noexcept { value += v.value; return *this; } \
      SUI_INLINE constexpr name & operator-=(const name &v) noexcept { value -= v.value; return *this; } \
      SUI_INLINE constexpr name & operator*=(const name &v) noexcept { value *= v.value; return *this; } \
      SUI_INLINE constexpr name & operator/=(const name &v) noexcept { value /= v.value; return *this; } \
      SUI_INLINE constexpr name & operator+=(const float v) noexcept { value += v; return *this; } \
      SUI_INLINE constexpr name & operator-=(const float v) noexcept { value -= v; return *this; } \
      SUI_INLINE constexpr name & operator*=(const float v) noexcept { value *= v; return *this; } \
      SUI_INLINE constexpr name & operator/=(const float v) noexcept { value /= v; return *this; } \
    }; \
    SUI_INLINE constexpr name operator+(const name &a, const name &b) noexcept { return name(a.value + b.value); } \
    SUI_INLINE constexpr name operator-(const name &a, const name &b) noexcept { return name(a.value - b.value); } \
    SUI_INLINE constexpr name operator*(const name &a, const name &b) noexcept { return name(a.value * b.value); } \
    SUI_INLINE constexpr name operator/(const name &a, const name &b) noexcept { return name(a.value / b.value); } \
    SUI_INLINE constexpr name operator+(const name &a, const float b) noexcept { return name(a.value + b); } \
    SUI_INLINE constexpr name operator-(const name &a, const float b) noexcept { return name(a.value - b); } \
    SUI_INLINE constexpr name operator*(const name &a, const float b) noexcept { return name(a.value * b); } \
    SUI_INLINE constexpr name operator/(const name &a, const float b) noexcept { return name(a.value / b); } \
    SUI_INLINE constexpr name operator+(const float a, const name &b) noexcept { return name(a + b.value); } \
    SUI_INLINE constexpr name operator-(const float a, const name &b) noexcept { return name(a - b.value); } \
    SUI_INLINE constexpr name operator*(const float a, const name &b) noexcept { return name(a * b.value); } \
    SUI_INLINE constexpr name operator/(const float a, const name &b) noexcept { return name(a / b.value); } \
    SUI_INLINE constexpr name operator-(const name &a) noexcept { return name(-a.value); } \
    SUI_INLINE constexpr name abs(const name &a) noexcept { return name(std::abs(a.value)); } \
    SUI_INLINE constexpr name max(const name &a, const name &b) noexcept { return name(std::max(a.value, b.value)); } \
    SUI_INLINE constexpr name min(const name &a, const name &b) noexcept { return name(std::min(a.value, b.value)); } \

    DE_INTEFACE_UNITS_LIST

#undef X

    struct unit {
      enum class type {
        not_specified,

#define X(name) name,
        DE_INTEFACE_UNITS_LIST
#undef X

        count
      };

      float value;
      enum type type;

      constexpr unit() noexcept : value(0.0f), type(type::not_specified) {}
      constexpr unit(const float value) noexcept : value(value), type(type::absolute) {}
      constexpr unit(const float value, const enum type type) noexcept : value(value), type(type) {}

#define X(name) constexpr unit(const name &a) noexcept : value(a.value), type(type::name) {}
      DE_INTEFACE_UNITS_LIST
#undef X

      // наверное нужно передать контекст, в нем должны храниться рассчитанные константы размеров экрана,
      // то есть необходимый мультипликатор для типа
      constexpr float get(const float mult = 1.0f) const noexcept { return value*mult; }
      float get(const context* ctx) const noexcept;

#define X(name) constexpr unit & operator=(const name &a) noexcept { value = a.value; type = type::name; return *this; }
      DE_INTEFACE_UNITS_LIST
#undef X

      constexpr ~unit() noexcept = default;
      constexpr unit(const unit &copy) noexcept = default;
      constexpr unit(unit &&move) noexcept = default;
      constexpr unit & operator=(const unit &copy) noexcept = default;
      constexpr unit & operator=(unit &&move) noexcept = default;
    };

    constexpr unit operator-(const unit &a) noexcept { return unit(-a.value, a.type); }

    namespace literals {
      using float_t = long double;
      using uint_t = unsigned long long;
      constexpr relative operator ""_pct(const float_t num) noexcept { return relative(num / 100.0); }
      constexpr relative operator ""_pct(const uint_t num) noexcept { return relative(float_t(num) / 100.0); }
      constexpr relative operator ""_frac(const float_t num) noexcept { return relative(num); }
      constexpr density_independent operator ""_dp(const float_t num) noexcept { return density_independent(num); }
      constexpr density_independent operator ""_dp(const uint_t num) noexcept { return density_independent(num); }
      constexpr absolute operator ""_px(const float_t num) noexcept { return absolute(num); }
      constexpr absolute operator ""_px(const uint_t num) noexcept { return absolute(num); }
      constexpr scaled_pixels operator ""_sp(const float_t num) noexcept { return scaled_pixels(num); }
      constexpr scaled_pixels operator ""_sp(const uint_t num) noexcept { return scaled_pixels(num); }
    }
  }
}

#endif
