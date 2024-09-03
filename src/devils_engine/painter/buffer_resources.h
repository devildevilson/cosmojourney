#ifndef DEVILS_ENGINE_PAINTER_BUFFER_RESOURCES_H
#define DEVILS_ENGINE_PAINTER_BUFFER_RESOURCES_H

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <reflect>
#include <atomic>
#include <concepts>
#include <type_traits>

namespace devils_engine {
namespace painter {

template<class Tuple>
struct remove_last;

template<>
struct remove_last<std::tuple<>>; // Define as you wish or leave undefined

template<class... Args>
struct remove_last<std::tuple<Args...>> {
private:
  using Tuple = std::tuple<Args...>;

  template<std::size_t... n>
  static std::tuple<std::tuple_element_t<n, Tuple>...> extract(std::index_sequence<n...>);
public:
  using type = decltype(extract(std::make_index_sequence<sizeof...(Args) - 1>()));
};

template<class Tuple>
using remove_last_t = typename remove_last<Tuple>::type;

template <typename T>
class clever_buffer {
public:
  using raw_data_t = decltype(reflect::to<std::tuple>(T{}));
  constexpr static const size_t element_count = std::tuple_size_v<raw_data_t>;
  static_assert(element_count > 0);
  using last_element_t = typename std::tuple_element_t<element_count-1, raw_data_t>;
  using data_part_t = typename std::conditional_t<std::is_pointer_v<last_element_t>, remove_last_t<raw_data_t>, raw_data_t>;
  using array_part_t = typename std::conditional_t<std::is_pointer_v<last_element_t>, std::remove_pointer_t<last_element_t>, void>;
  constexpr static const size_t at_least_size = utils::align_to(sizeof(data_part_t), 16) + sizeof(array_part_t);

  using counter_t = uint32_t;

  clever_buffer() noexcept : counter(0), mapped_pointer(nullptr) {}

  template <std::same_as<array_part_t> U = array_part_t>
    requires (!std::is_same_v<last_element_t, void>)
  void* interpretate_place(const counter_t index) {
    //if constexpr (std::is_same_v<array_part_t, void>) return nullptr;
    auto ptr = reinterpret_cast<char*>(mapped_pointer);
    ptr += sizeof(data_part_t);
    ptr += index * sizeof(array_part_t);
    return ptr;
  }

  template <typename... Args>
    requires (!std::same_as<last_element_t, void>)
  counter_t add(Args&&... args) {
    const auto index = counter.fetch_add(1);
    auto ptr = interpretate_place(index);
    new (ptr) T{std::forward<Args>(args)...};
    return index;
  }

  template <std::same_as<array_part_t> U = array_part_t>
    requires (!std::is_same_v<last_element_t, void>)
  array_part_t & arr_at(const counter_t index) {
    auto ptr = interpretate_place(index);
    return reinterpret_cast<array_part_t*>(ptr);
  }

  template <std::same_as<array_part_t> U = array_part_t>
    requires (!std::is_same_v<last_element_t, void>)
  const array_part_t & arr_at(const counter_t index) const {
    auto ptr = interpretate_place(index);
    return reinterpret_cast<array_part_t*>(ptr);
  }

  template <size_t N>
  auto & get() { 
    return reflect::get<N>(*reinterpret_cast<T*>(mapped_pointer));
  }

  template <size_t N>
  const auto & get() const { 
    return reflect::get<N>(*reinterpret_cast<T*>(mapped_pointer));
  }

  template <reflect::fixed_string Name>
  auto & get() { 
    return reflect::get<Name>(*reinterpret_cast<T*>(mapped_pointer));
  }

  template <reflect::fixed_string Name>
  const auto & get() const { 
    return reflect::get<Name>(*reinterpret_cast<T*>(mapped_pointer));
  }

  auto & get() { return *reinterpret_cast<T*>(mapped_pointer); }
  const auto & get() const { return *reinterpret_cast<T*>(mapped_pointer); }
protected:
  std::atomic<counter_t> counter;
  void* mapped_pointer;
};

// значицца в чем прикол:
// практически все буферы на гпу имеют примерно схожую сигнатуру
// struct_t { elem1, elem2, elem3, elem4* };
// где последний элемент можно сделать указателем чтобы тот смотрел на остаток буфера
// эту сигнатуру удобно выводить средствами reflect
// из нее можно получить собственно удачный маппинг между хостом и гпу
// из нее так же можно составить объявление в шейдер (большую часть шейдера можно будет не переписывать по миллиарду раз)
template <typename T>
class storage_buffer : public clever_buffer<T> {
public:
  
protected:
  VkDevice device;
  VkBuffer buffer;
};

}
}

#endif