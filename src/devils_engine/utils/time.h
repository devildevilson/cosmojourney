#ifndef DEVILS_ENGINE_UTILS_TIME_H
#define DEVILS_ENGINE_UTILS_TIME_H

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <string>

namespace devils_engine {
namespace utils {
  using timestamp_t = size_t;

  class time_log {
  public:
    std::string str;
    std::chrono::steady_clock::time_point tp;

    time_log(std::string str) noexcept;
    ~time_log() noexcept;

    time_log(const time_log &copy) noexcept = delete;
    time_log(time_log &&move) noexcept = delete;
    time_log & operator=(const time_log &copy) noexcept = delete;
    time_log & operator=(time_log &&move) noexcept = delete;
  };

  // возвращаем unix timestamp (наверное будет приходить по локальному времени...)
  constexpr timestamp_t timestamp() noexcept {
    const auto p1 = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
  }

  template <typename F, typename... Args>
  auto perf(std::string msg, F f, Args&&... args) {
    time_log l(std::move(msg));
    return f(std::forward<Args>(args)...);
  }

  // все значения будут начинаться с нуля
  // в игровом понимании было бы неплохо указать некую стартовую дату
  // и уметь соответствено приводить от одного к другому
  // нужно уметь вычитать и складывать
  // при этом в игре у нас потенциально могут быть заданы иные правила для игрового дня
  struct date {
    int32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    // день недели, блен сильно зависит какой был день недели в самом начале, но при этом можно от этого вывести какой сейчас день недели
    uint32_t week_day;

    date();

    // не можем так складывать и вычитать
    //date &operator+=(const date &b) noexcept;
    //date &operator-=(const date &b) noexcept;
  };

  // наверное наиболее точным форматом будет указать количество секунд в году
  // и от этого отталкиваться
  class date_system {
  public:
    struct month {
      std::string name;
      uint32_t days;
      int32_t leap_day; // years
    };

    date_system() noexcept;
    void init(std::vector<month> months, std::vector<std::string> week, const uint32_t hours_in_day = 24, const uint32_t minutes_in_hour = 60, const uint32_t seconds_in_minute = 60) noexcept;
    void set_start_date(const date &d) noexcept;

    bool is_valid(const date &d) const;
    timestamp_t cast(const date &d) const;
    date cast(const timestamp_t &t) const;
    timestamp_t add(const timestamp_t cur, int32_t years, int32_t months, int32_t days) const;

    date initial_date() const;
    std::string_view month_str(const uint32_t index) const;
    std::string_view month_str(const date &d) const;
    std::string_view week_day_str(const uint32_t index) const;
    std::string_view week_day_str(const date &d) const;
    bool is_leap_year(const int32_t year) const;
    bool is_leap_year(const date &d) const;

    size_t date_count_days(const date &d) const;
    size_t week_number(const date &d) const;
  private:
    size_t year_days(const int32_t year) const;
    size_t day_seconds() const;

    date start_date;
    std::vector<month> months;
    std::vector<std::string> week;
    uint32_t hours_in_day;
    uint32_t minutes_in_hour;
    uint32_t seconds_in_minute;
  };

  class game_date {
  public:
    static const date_system & get();
    void init(std::vector<date_system::month> months, std::vector<std::string> week, const uint32_t hours_in_day = 24, const uint32_t minutes_in_hour = 60, const uint32_t seconds_in_minute = 60);
    void set_start_date(const date &d);
  private:
    static date_system s;
  };

  // так мне еще нужна структура которая посчитает относительное время и этим временем будут пользоваться все остальные системы в приложении
  // для этого времени есть пауза в меню (для одиночной игры)
  // какое разрешение у timestamp_t? удобно брать микросекунды
  struct app_clock {
    timestamp_t start_time;
    timestamp_t elapsed_time;

    // mcs
    static size_t resolution();

    app_clock(const timestamp_t start_time) noexcept;
    void update(const size_t delta);

    timestamp_t timestamp() const;
    int64_t diff(const timestamp_t timestamp) const;
    double seconds() const;
  };
}
}

#endif