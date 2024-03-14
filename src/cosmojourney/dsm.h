#ifndef COSMOJOURNEY_DSM_H
#define COSMOJOURNEY_DSM_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <array>

namespace cosmojourney {
  class dsm {
  public:
    dsm() noexcept = default;
    ~dsm() noexcept = default;
    dsm(const dsm &copy) noexcept = default;
    dsm(dsm &&move) noexcept = default;
    dsm & operator=(const dsm &copy) noexcept = default;
    dsm & operator=(dsm &&move) noexcept = default;

    std::string_view process_event(const std::string_view &current_state, const std::string_view &event, void* obj);
    void parse_table_line(std::string line);
  private:
    struct transition {
      std::string_view current_state;
      std::string_view event;
      std::string_view final_state;
      std::array<std::string_view, 8> guards;
      std::array<std::string_view, 8> actions;
      // наверное нужно как то указать где заканчивается все штуки по текущему стейту?
      size_t next_state;

      transition() noexcept = default;
      ~transition() noexcept = default;
      transition(const transition &copy) noexcept = default;
      transition(transition &&move) noexcept = default;
      transition & operator=(const transition &copy) noexcept = default;
      transition & operator=(transition &&move) noexcept = default;
    };

    std::vector<transition> table;
    std::vector<std::string> memory;

    size_t find_transition(const std::string_view &state, const std::string_view &event) const;
    void recompute_next_states();
  };
}

#endif