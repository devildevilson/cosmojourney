#include "time.h"
#include "core.h"

namespace devils_engine {
namespace utils {
  time_log::time_log(std::string str) noexcept : str(std::move(str)), tp(std::chrono::steady_clock::now()) {}
  time_log::~time_log() noexcept {
    const auto dur = std::chrono::steady_clock::now() - tp;
    const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
    utils::info("{} took {} mcs ({:.3} seconds)", str, mcs, double(mcs)/1000000.0);
  }

  date::date() : year(0), month(0), day(0), hour(0), minute(0), second(0) {}

  static const std::vector<date_system::month> default_months({
    date_system::month{ "january", 31, 0 },
    date_system::month{ "february", 28, 4 },
    date_system::month{ "march", 31, 0 },
    date_system::month{ "april", 31, 0 },
    date_system::month{ "may", 31, 0 },
    date_system::month{ "june", 31, 0 },
    date_system::month{ "july", 31, 0 },
    date_system::month{ "august", 31, 0 },
    date_system::month{ "september", 31, 0 },
    date_system::month{ "october", 31, 0 },
    date_system::month{ "november", 31, 0 },
    date_system::month{ "december", 31, 0 }
  });

  date_system::date_system() noexcept : months(default_months), hours_in_day(24), minutes_in_hour(60), seconds_in_minute(60), days_in_week(7) {}
  void date_system::init(std::vector<month> months, const uint32_t hours_in_day, const uint32_t minutes_in_hour, const uint32_t seconds_in_minute, const uint32_t days_in_week) noexcept {
    this->months = std::move(months);
    this->hours_in_day = hours_in_day;
    this->minutes_in_hour = minutes_in_hour;
    this->seconds_in_minute = seconds_in_minute;
    this->days_in_week = days_in_week;
  }

  void date_system::set_start_date(const date &d) noexcept {
    this->start_date = d;
  }

  //timestamp_t date_system::cast(const date &d) const {}

  static int32_t sign(const double a) {
    return a >= 0 ? 1 : -1;
  }

  // недостаточная проверка валидности !!!!!!!!! но будет работать если задавать только стартовый год
  bool date_system::is_valid(const date &d) const {
    if (d.year < start_date.year) return false;
    if (d.month >= months.size()) return false;
    const auto &m = months[d.month];
    const uint32_t month_days = m.days + sign(m.leap_day) * int32_t(d.year % std::abs(m.leap_day) == 0);
    if (d.day >= month_days) return false;
    if (d.hour >= hours_in_day) return false;
    if (d.minute >= minutes_in_hour) return false;
    if (d.second >= seconds_in_minute) return false;
    return true;
  }

  timestamp_t date_system::cast(const date &d) const {
    // все таки будет очень удобно сделать каст из даты в таймстамп
    // как? нужно узнать количество дней до даты
    if (!is_valid(d)) return SIZE_MAX;

    auto cur = start_date;
    size_t days = 0;
    while (cur.year <= d.year || cur.month <= d.month || cur.day < d.day) {
      if (cur.month >= months.size()) {
        cur.month = 0;
        cur.year += 1;
      }

      const auto &m = months[cur.month];
      const uint32_t month_days = m.days + sign(m.leap_day) * int32_t(m.leap_day != 0 && (cur.year % std::abs(m.leap_day) == 0));
      if (cur.year == d.year || cur.month == d.month) {
        days += size_t(d.day) - cur.day;
        cur.day = d.day;
      } else {
        const uint32_t remain_days = month_days - cur.day;
        cur.day = 0;
        days += remain_days;
        cur.month += 1;
      }
    }

    const size_t seconds_in_day = day_seconds();
    const size_t prev_day_seconds = size_t(cur.hour) * minutes_in_hour * seconds_in_minute + size_t(cur.minute) * seconds_in_minute + cur.second;
    const size_t cur_day_seconds = size_t(d.hour) * minutes_in_hour * seconds_in_minute + size_t(d.minute) * seconds_in_minute + d.second;
    size_t seconds_part = 0;
    if (prev_day_seconds > cur_day_seconds) {
      days -= 1;
      seconds_part = seconds_in_day - (prev_day_seconds - cur_day_seconds);
    } else {
      seconds_part = cur_day_seconds - prev_day_seconds;
    }
    
    const timestamp_t t = (days * hours_in_day * minutes_in_hour * seconds_in_minute + seconds_part) * app_clock::resolution();
    return t;
  }

  date date_system::cast(const timestamp_t &t) const {
    auto d = start_date;
    const size_t seconds_in_day = day_seconds();
    // начнем с того что посчитаем всего секунд
    const size_t stamp_seconds = double(t) / double(app_clock::resolution());
    // теперь нужно прибавить секунды к стартовой дате
    const size_t seconds_part = stamp_seconds % seconds_in_day;
    const size_t add_seconds = seconds_part % seconds_in_minute;
    const size_t minutes_part = seconds_part / minutes_in_hour;
    size_t add_minutes = minutes_part % minutes_in_hour;
    size_t hours_part = minutes_part / hours_in_day;
    size_t add_days = double(stamp_seconds) / double(seconds_in_day);

    d.second += add_seconds;
    if (d.second >= seconds_in_minute) {
      d.second -= seconds_in_minute;
      d.minute += 1;
    }

    d.minute += add_minutes;
    if (d.minute >= minutes_in_hour) {
      d.minute -= minutes_in_hour;
      d.hour += 1;
    }

    d.hour += hours_part;
    if (d.hour >= hours_in_day) {
      d.hour -= hours_in_day;
      add_days += 1;
    }

    // прибавляем дни
    while (add_days != 0) {
      if (d.month >= months.size()) {
        d.month = 0;
        d.year += 1;
      }

      const auto &m = months[d.month];
      const uint32_t month_days = m.days + sign(m.leap_day) * int32_t(m.leap_day != 0 && (d.year % std::abs(m.leap_day) == 0));
      const uint32_t remain_days = month_days - d.day;
      if (add_days <= remain_days) {
        d.day += add_days;
        add_days = 0;
      } else {
        d.day = 0;
        d.month += 1;
        add_days -= remain_days;
      }
    }

    // этого по идее не может быть
    if (d.month >= months.size()) {
      d.month = 0;
      d.year += 1;
    }

    return d;
  }

  // чутка долго получается, как быстрее? сразу пачками прибавлять/вычитать
  // там увы не очень очевидно как сделать для двух знаков
  // прям супер тяжелая функция
  timestamp_t date_system::add(const timestamp_t cur, const int32_t years, int32_t months, int32_t days) const {
    auto cur_date = cast(cur);

    while (days != 0) {
      days = sign(days) * int32_t(std::abs(days) - 1);
      cur_date.day += sign(days);

      const auto &m = this->months[cur_date.month];
      const uint32_t month_days = m.days + sign(m.leap_day) * int32_t(m.leap_day != 0 && (cur_date.year % std::abs(m.leap_day) == 0));

      if (cur_date.day == UINT32_MAX) {
        cur_date.day = month_days-1;
        cur_date.month -= 1;
      }

      if (cur_date.day >= month_days) {
        cur_date.day = 0;
        cur_date.month += 1;
      }

      if (cur_date.month == UINT32_MAX) {
        cur_date.month = this->months.size()-1;
        cur_date.year -= 1;
      }

      if (cur_date.month >= this->months.size()) {
        cur_date.month = 0;
        cur_date.year += 1;
      }
    }

    while (months != 0) {
      months = sign(months) * int32_t(std::abs(months) - 1);
      cur_date.month += sign(months);
      if (cur_date.month == UINT32_MAX) {
        cur_date.month = this->months.size()-1;
        cur_date.year -= 1;
      }

      if (cur_date.month >= this->months.size()) {
        cur_date.month = 0;
        cur_date.year += 1;
      }
    }

    cur_date.year += years;

    return cast(cur_date);
  }

  size_t date_system::year_days(const int32_t year) const {
    size_t days = 0;
    for (const auto &m : months) {
      const uint32_t month_days = m.days + sign(m.leap_day) * int32_t(m.leap_day != 0 && (year % std::abs(m.leap_day) == 0));
      days += month_days;
    }

    return days;
  }

  size_t date_system::day_seconds() const {
    return size_t(hours_in_day) * size_t(minutes_in_hour) * size_t(seconds_in_minute);
  }

  const date_system &game_date::get() { return s; }
  void game_date::init(std::vector<date_system::month> months, const uint32_t hours_in_day, const uint32_t minutes_in_hour, const uint32_t seconds_in_minute, const uint32_t days_in_week) {
    s.init(months, hours_in_day, minutes_in_hour, seconds_in_minute, days_in_week);
  }

  void game_date::set_start_date(const date &d) { s.set_start_date(d); }

  date_system game_date::s;

  size_t app_clock::resolution() { return 1000000; }

  app_clock::app_clock(const timestamp_t start_time) noexcept : start_time(start_time), elapsed_time(0) {}
  void app_clock::update(const size_t delta) { elapsed_time += delta; }

  timestamp_t app_clock::timestamp() const { return start_time + elapsed_time; }
  int64_t app_clock::diff(const timestamp_t timestamp) const {
    return this->timestamp() < timestamp ? timestamp - this->timestamp() : -(this->timestamp() - timestamp);
  }

  double app_clock::seconds() const { return double(this->timestamp()) / double(resolution()); }
}
}