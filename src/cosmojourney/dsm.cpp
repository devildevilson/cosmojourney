#include "dsm.h"
#include <utils/core.h>
#include <utils/string.h>

using namespace devils_engine;

namespace cosmojourney {
  std::string_view dsm::process_event(const std::string_view &current_state, const std::string_view &event, void* obj) {
    size_t index = find_transition(current_state, event);
    if (index == SIZE_MAX || (current_state != table[index].current_state || event != table[index].event)) 
      utils::error("Could not find any transition from state '{}' using event '{}'", current_state, event);
    const size_t end = table[index].next_state;
    for (; index < end; ++index) {
      bool guard_pass = true;
      const auto &g = table[index].guards;
      for (size_t i = 0; g[i] != "" && i < g.size(); ++i) {
        // гвард
        utils::println("guard", i, g[i]);
      }

      if (!guard_pass) continue;
      const auto &a = table[index].actions;
      for (size_t i = 0; a[i] != "" && i < a.size(); ++i) {
        // действие
        utils::println("action", i, a[i]);
      }

      const auto ret = table[index].final_state == "" ? current_state : table[index].final_state;
      return ret;
    }

    // если попадаем сюда то что? перехода по идее просто не произойдет, может это ворнинг?
    utils::warn("Could not find proper transition from state '{}' using event '{}'", current_state, event);

    return "";
  }

  static void parse_table_line_impl(
    const std::string_view &line, 
    std::string_view &current_state, 
    std::string_view &event, 
    // было бы неплохо тут вернуть количество, с другой стороны мы можем проверить пустую строку
    std::string_view *guard_list, const size_t max_guards, 
    std::string_view *action_list, const size_t max_actions, 
    std::string_view &final_state
  ) {
    const size_t mem_size = 8;
    std::array<std::string_view, mem_size> mem;
    size_t count = 0;
    count = utils::string::split(line, "=", mem.data(), mem_size);
    assert(count <= 2);
    const auto left_value = utils::string::trim(mem[0]);
    final_state = utils::string::trim(mem[1]);
    count = utils::string::split(left_value, "->", mem.data(), mem_size);
    assert(count <= 2);
    const auto before_action = utils::string::trim(mem[0]);
    const auto actions = utils::string::trim(mem[1]);
    count = utils::string::split(before_action, "+", mem.data(), mem_size);
    assert(count == 2);
    current_state = utils::string::trim(mem[0]);
    const auto event_and_guards = utils::string::trim(mem[1]);
    event = utils::string::trim(event_and_guards.substr(0, event_and_guards.find("[")));
    const auto guards = utils::string::trim(utils::string::inside(event_and_guards, "[", "]"));
    count = utils::string::split(guards, ",", guard_list, max_guards);
    assert(count < max_guards);
    for (size_t i = 0; i < count; ++i) guard_list[i] = utils::string::trim(guard_list[i]);
    count = utils::string::split(actions, ",", action_list, max_actions);
    assert(count < max_actions);
    for (size_t i = 0; i < count; ++i) action_list[i] = utils::string::trim(action_list[i]);
  }

  void dsm::parse_table_line(std::string line) {
    memory.push_back(std::move(line));
    transition t;
    parse_table_line_impl(memory.back(), t.current_state, t.event, t.guards.data(), 8, t.actions.data(), 8, t.final_state);
    auto itr = table.begin();
    for (; itr->current_state == t.current_state && itr != table.end(); ++itr) {}
    table.insert(itr, t);
    recompute_next_states();
  }

  size_t dsm::find_transition(const std::string_view &state, const std::string_view &event) const {
    size_t i = 0;
    for (; i < table.size() && state != table[i].current_state; i = table[i].next_state) {}
    if (i >= table.size()) return SIZE_MAX;
    for (; event != table[i].event && i < table[i].next_state; ++i) {}
    //if (state != table[i].current_state || event != table[i].event) return SIZE_MAX;
    return i;
  }

  void dsm::recompute_next_states() {
    // предполагаем что вещи уже сгруппированны как надо
    auto cur_state = table[0].current_state;
    size_t index = 0;
    while (index < table.size()) {
      size_t next_index = index;
      for (; cur_state == table[next_index].current_state && next_index < table.size(); ++next_index) {}
      for (; index < next_index; ++index) {
        table[index].next_state = next_index;
      }
      index = next_index;
    }
  }
}