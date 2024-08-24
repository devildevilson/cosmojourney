#ifndef DEVILS_ENGINE_INPUT_EVENTS_H
#define DEVILS_ENGINE_INPUT_EVENTS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>

namespace devils_engine {
namespace input {

// что такое ивенты? это некое событие по нажатию кнопки
// события скорее всего обрабатываются только момент обновления системы
// и представляют собой именнованную сущность (слой абстракции над кнопками)
// обычно в играх на одно событие приходится 2 кнопки
// я так понимаю у меня будет два типа событий:
// заданы по умолчанию мной + пришедшие из модов

namespace default_events {
enum values {

};
}

namespace event_state {
enum values {
  release      = 0,
  press        = 0b1,
  long_press   = 0b10,
  click        = 0b100,
  long_click   = 0b1000,
  double_press = 0b10000,
  double_click = 0b100000,
};
}

// вообще было бы неплохо получать это дело из настроек
constexpr size_t long_press_duration = 300000;
constexpr size_t double_press_duration = 400000;

// так тут еще как минимум есть инпут маппинг
// то есть несколько таблиц инпутов для разных ситуаций
// например инпут в меню мы должны только в меню обрабатывать
// (в меню на стрелки например переключение между пунктами меню)
// (в игре передвижение в машине и без - разные инпуты)
// но при этом я бы не сказал что хорошая идея разграничивать это дело как жеско в коде
// например какие то вещи могут пересекаться
// имеет ли смысл использовать тут сканкоды?
// вообще имеет смысл для пользователя
struct events {
  struct event_map {
    std::array<int32_t, 16> keys;
  };

  struct key_state {
    event_state::values current;
    event_state::values prev;
    size_t time;
    std::array<std::string_view, 16> events;
  };

  
private:
  // контейнер для кнопок
  // контейнер для ивентов
  // контейнер для уникальных ивентов
  
};
}
}

#endif
