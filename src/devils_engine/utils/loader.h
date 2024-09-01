#ifndef DEVILS_ENGINE_UTILS_LOADER_H
#define DEVILS_ENGINE_UTILS_LOADER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include "thread/lock.h"
#include "load_stage.h"

namespace devils_engine {
namespace utils {

class loader : public thread::semaphore_interface {
public:
  std::string name;
  
  loader(std::string name) noexcept;
  virtual ~loader() noexcept = default;
  
  // не синхронизировано
  template <typename T, typename... Args>
  T* add(Args&&... args) {
    _stages.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return _stages.back().get();
  }

  void process();

  size_t counter() const noexcept;
  size_t size() const noexcept;
  bool finished() const noexcept;
  // здесь мы не должны выпасть за пределы массива
  std::string_view stage_name() const noexcept;

  void reset() override;
  thread::semaphore_state::values state() const override;
  bool wait_until(tp t, const size_t tolerance_in_ns = 1) const override;
protected:
  std::atomic<size_t> _counter;
  std::vector<std::unique_ptr<load_stage>> _stages;
};

class loader2 {
public:
  using lock = std::unique_lock<std::mutex>;

  std::string name;
  
  loader2(std::string name) noexcept;
  ~loader2() noexcept = default;
  
  template <typename T, typename... Args>
  T* add(Args&&... args) {
    lock l(_mutex);
    _stages.push_back(std::make_unique<T>(std::forward<Args>(args)...));
  }

  void process();

  size_t counter() const noexcept;
  size_t size() const noexcept;
  bool finished() const noexcept;
  std::string_view stage_name() const noexcept;
protected:
  mutable std::mutex _mutex;
  load_stage* _stage;
  size_t _counter;
  std::vector<std::unique_ptr<load_stage>> _stages;
};
}
}

#endif