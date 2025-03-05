#ifndef DEVILS_ENGINE_SIMUL_INTERFACE_H
#define DEVILS_ENGINE_SIMUL_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <mutex>

// так ну это просто каркас системы которая постоянно апдейтит свое состояние
// как системы правильно синхронизировать между собой? возможно это какой то еще дополнительный механизм сверху
// с другой стороны мы можем в апдейтере определить мьютекс
// и не делать апдейт пока мьютекс забран, работает ли это универсально?
// кажется что решение хорошее, но работает не полностью универсально 
// система загрузки ресурсов как должна выглядеть?
// что то вроде тред пула (или возможно он и есть), положим последовательно загрузчики
// в пул, между загрузчиками есть синхронизация такая что зависимый загрузчик запустится только после предыдущего
// с системами будут активно общаться только Главная система + загрузчик
// всем остальным ожидаемо все равно на другие системы
// в большинстве случаев система НЕ ДОЛЖНА выходить за пределы той памяти которую ей обозначил загрузчик
// как только загрузчик скажет что этот участок памяти доступен все остальные ребята в системе должны
// получить возможность им пользоваться, а до этого системы не должны лезть никуда дальше бай дизайн
// в плане использования ПОЧТИ всех ресурсов ГПУ - у нас есть дескриптор, 
// обновление которого позволяет получить или закрыть доступ к участку памяти
// + к этому у нас есть некоторое количество моделек которые попадают в отдельный рендер
// и надо бы иметь возможность отключать и включать возможность их отрисовки на экране
// в плане памяти ЦПУ - чуть сложнее история
// нет каких то централизованных контейнеров ресурсов и разные моменты времени мы будем пытаться 
// получить доступ к тому чего нет... что в этом случае?
// заранее создать некоторое количество прокси объектов которые подскажут статус ресурса
// следить за состоянием загрузчика - как то дать понять ресурсу что он загружается
// например у нас не прогружены все анимации, и вот мы видим какой то объект
// где используется недозагруженная анимация, как понять что ресурса нет?
// как понять что он сейчас грузится еще? тоже самое со звуками
// пока что не понимаю

namespace devils_engine {
namespace simul {
class interface {
public:
  virtual ~interface() noexcept = default;
  virtual void init() = 0;
  virtual bool stop_predicate() const = 0;
  virtual void update(const size_t time) = 0;
};

class advancer : public interface {
public:
  using super = interface;
  using clock_t = std::chrono::high_resolution_clock;

  advancer() noexcept;
  virtual ~advancer() noexcept = default;

  virtual void run(const size_t wait_mcs); // interface functions under the mutex

  size_t frame_time() const;
  size_t counter() const;
  clock_t::time_point start() const;
  double compute_fps() const;
  void set_frame_time(const size_t frame_time);
  void reset_counter();
  void stop();

  inline std::mutex & acquire_sync_object() const { return mutex; }
  bool stop_predicate() const override;
 protected:
  mutable std::mutex mutex;
  size_t _frame_time;
  size_t new_frame_time;
  size_t _counter;
  clock_t::time_point _start;
  bool _stop;
};
}
}

#endif