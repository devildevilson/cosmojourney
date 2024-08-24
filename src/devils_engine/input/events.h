#ifndef DEVILS_ENGINE_INPUT_EVENTS_H
#define DEVILS_ENGINE_INPUT_EVENTS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <array>
#include <chrono>
//#include <utils/flat_hash_map.hpp>
#include <parallel_hashmap/phmap.h>

namespace devils_engine {
namespace input {

// что такое ивенты? это некое событие по нажатию кнопки
// события скорее всего обрабатываются только момент обновления системы
// и представляют собой именнованную сущность (слой абстракции над кнопками)
// обычно в играх на одно событие приходится 2 кнопки
// я так понимаю у меня будет два типа событий:
// заданы по умолчанию мной + пришедшие из модов

#define DEVILS_ENGINE_INPUT_DEFAULT_EVENTS_LIST \
  X(action_key) \


namespace default_events {
enum values {
#define X(name) name,
  DEVILS_ENGINE_INPUT_DEFAULT_EVENTS_LIST
#undef X

  count
};

constexpr auto names = {
#define X(name) #name,
  DEVILS_ENGINE_INPUT_DEFAULT_EVENTS_LIST
#undef X
};
}

namespace key_state {
enum values {
  release,
  press,
  repeated
};
}

// приоритет от большего к меньшему
namespace event_state {
enum values {
  release      = 0,
  press        = 0b000001,
  long_press   = 0b000010,
  click        = 0b000100,
  long_click   = 0b001000,
  double_press = 0b010000,
  double_click = 0b100000,
};
}

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
public:
  struct event_map {
    std::array<int32_t, 4> keys;

    inline event_map() noexcept : keys{-1} {}
  };

  struct key_data {
    event_state::values current;
    event_state::values prev;
    key_state::values state;
    size_t event_time;
    size_t key_time;
    std::array<std::string_view, 16> events;
    int32_t glfw_key;

    inline key_data() : current(event_state::values::release), prev(event_state::values::release), state(key_state::values::release), event_time(0), key_time(0), glfw_key(-1) {}
  };

  struct auxiliary {
    static constexpr size_t text_memory_size = 256;

    float mouse_wheel;
    uint32_t current_text;
    uint32_t text[text_memory_size];
    size_t last_frame_time;

    std::chrono::steady_clock::time_point double_click_time_point;
    int32_t click_pos_x, click_pos_y;
    float fb_scale_x, fb_scale_y;

    inline auxiliary() : mouse_wheel(0.0f), current_text(0), text{0}, last_frame_time(0), click_pos_x(0), click_pos_y(0), fb_scale_x(0.0f), fb_scale_y(0.0f) {}
  };

  static void init();
  static void update(const size_t time);
  static void update_key(const int32_t scancode, const int32_t state);
  static void set_key(const std::string_view &id, const int32_t scancode, const int32_t key = -1, const uint8_t slot = 0);

  static bool is_pressed(const std::string_view &id);
  static bool is_released(const std::string_view &id);

  static std::string_view key_name(const int32_t key, const int32_t scancode);
  static std::string_view key_name(const std::string_view &id, const uint8_t slot);
  static const std::array<std::string_view, 16> & mapping(const int32_t scancode);
  static bool check_key(const int32_t scancode, const uint32_t states);
  static bool timed_check_key(const int32_t scancode, const uint32_t states, const size_t wait, const size_t period);
  static bool check_event(const std::string_view &event, const uint32_t states);
  static bool timed_check_event(const std::string_view &event, const uint32_t states, const size_t wait, const size_t period);

  static void set_long_press_duration(const size_t dur);
  static void set_double_press_duration(const size_t dur);
  static void set_engine_tick_time(const size_t time);

  static std::string_view make_persistent_event_id(const std::string_view &id);

  static struct auxiliary &auxiliary_data();
 private:
  static size_t time;
  static size_t long_press_duration;
  static size_t double_press_duration;
  static size_t engine_tick_time;
  static struct auxiliary auxiliary;
  static phmap::flat_hash_map<std::string_view, event_map> event_mapping;
  static phmap::flat_hash_map<int32_t, key_data> key_mapping;  // scancodes
  static phmap::flat_hash_set<std::string> additional_events;
};
}
}

#endif
