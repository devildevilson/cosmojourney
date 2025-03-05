#ifndef DEVILS_ENGINE_SIMUL_WORLD_H
#define DEVILS_ENGINE_SIMUL_WORLD_H

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <memory>
#include <thread>

#include "interface.h"

namespace devils_engine {
namespace simul {

class color;
class music;

// тут мы бы хотели запустить рядом звуки и рендер
class world : public advancer {
public:
  world();
  void init() override;
  void update(const size_t time) override;
  bool stop_predicate() const override;
private:
  // поток для рендера, поток для звуков,
  // здесь создаем системы интерфейса + геймплея
  std::unique_ptr<color> render;
  std::unique_ptr<music> sound;
  std::jthread t1;
  std::jthread t2;
};

class color : public advancer {
public:
  color();
  void init() override;
  void update(const size_t time) override;
  bool stop_predicate() const override;
private:
  // было бы неплохо создать весь стейт отрисовки так чтобы он конфиг
  // подтягивал в рантайме и на него реагировал, то есть что то вроде двойной буферизации конфига
  // а где будет создано окно? вообще наверное все таки в world, а от него в color передадим
  // сюрфейс, по идее это будет функция в которой как раз закроем использование окна в world
  // и быстренько создадим surface
};

class music : public advancer {
public:
  music();
  void init() override;
  void update(const size_t time) override;
  bool stop_predicate() const override;
private:
  // короче нам нужно соблюсти несколько условий:
  // 1. данные звуков как то должны понимать информацию между кадрами и их можно тем или иным образом сопровождать
  // 2. данные звуков получаем из другого потока
  // есть ли необходимость городить схемы похожие на двойную буферизацию в этом случае?
  // вообще я бы не сказал что в этом есть особый смысл именно у звуков
  // мне тут нужна система двойной буферизации, но с обратной связью
  // ну типа можно вот что - используем 2 буфера с обоих сторон
  // в эти буферы записываем изменения итового буфера
  // после записи изменений записываем все в итоговый буфер
};

}
}

#endif