#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <ranges>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <cstring>
#include <cassert>

#include "sound/system.h"
#include "utils/core.h"
#include "utils/type_traits.h"
#include "utils/bitstream.h"

using namespace devils_engine;

void func1(std::vector<void*> &vec) {
  utils::time_log log("remove from middle");

  const size_t size = vec.size();
  while (vec.size() >= size / 2) {
    vec.erase(vec.begin()+34);
  }
}

void func2(std::vector<void*> &vec) {
  utils::time_log log("remove from end");

  const size_t size = vec.size();
  while (vec.size() >= size / 2) {
    vec.pop_back();
  }
}

void load_file(const std::string &file_name, std::vector<char> &buffer) {
  std::ifstream file(file_name);
  if (!file) throw std::runtime_error("Could not load file " + file_name);
  //if (!file.is_open()) throw std::runtime_error("Could not load file " + file_name);

  // file.seekg(0,std::ios::end);
  // const size_t size = file.tellg();
  // file.seekg(0, std::ios::beg);
  //
  // file.read(&buffer[0], size);
  buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void traced_func(const utils::tracer = utils::tracer()) {
  utils::info("aaaaa");
}

// было бы неплохо аннотировать как то функции для трассы
// хотя поди теперь когда тут нормально запускается gdb может быть и не нужно
// над запустить плохой ассерт
void traced_func2() {
  const utils::tracer t;
  utils::info("aaaaa");
}

void traced_func3() {
  utils::info("aaaaa");
  //utils_assertf(5==4, "test assert {}", 1);
}

int main(int argc, char const *argv[]) {
  //const size_t test_count = 100000;

  // std::random_device rd;
  // std::vector<void*> vec(test_count, nullptr);
  // std::for_each(vec.begin(), vec.end(), [&rd] (void* &data) { const size_t val = size_t(rd()) << 32 | rd(); data = reinterpret_cast<void*>(val); });
  //
  // func1(vec);
  // func2(vec);

  // мне бы хотелось сильно переработать то как у меня отрисовывается кадр
  // прежде всего я бы хотел готовить сам рендер в самом начале кадра, а затем пока он вычисляется
  // вычислять собственно инпут и изменения состояния игры

  // что то вроде:
  // while (true) {
  //   timer.start();

  //   render->update();
  //   render->start();

  //   user_input();
  //   thinker(); // ИИ и физику наверное одновременно сделать не выйдет
  //   physics(); // + было бы неплохо сразу из коробки клиент-серверную архитектуру, результаты вычисления нужно сравнить с сервером
  //   wait_when_it_save_to_push(render);
  //   push_to_render(); // вот тут может быть целая куча мелких дел которые можно по разным потокам раскидать

  //   render->end();
  //   render->present();

  //   timer.end();
  //   sync(timer);
  // }

  // sound::system s;
  // {
  //   const std::string file_name = "beautifulopen-your-gate-hiroyuki-sawano.mp3";
  //   utils::time_log log("loading resource " + file_name);
  //   std::vector<char> buffer;
  //   load_file(file_name, buffer);
  //   s.load_resource("test_mp3", sound::system::resource::type::mp3, std::move(buffer));
  // }
  //
  // {
  //   const std::string file_name = "alien-snake-roars.ogg";
  //   utils::time_log log("loading resource " + file_name);
  //   std::vector<char> buffer;
  //   load_file(file_name, buffer);
  //   s.load_resource("test_ogg", sound::system::resource::type::ogg, std::move(buffer));
  // }
  //
  // {
  //   const std::string file_name = "lofi-piano-loop.wav";
  //   utils::time_log log("loading resource " + file_name);
  //   std::vector<char> buffer;
  //   load_file(file_name, buffer);
  //   s.load_resource("test_wav", sound::system::resource::type::wav, std::move(buffer));
  // }
  //
  // {
  //   utils::time_log log("start playing"); // чет долго ужс, надо первую загрузку спихнуть в апдейт
  //   s.play_sound("test_ogg", 0, 1.0f, 0.4f);
  //   // если звук далеко находится то его тоже играть не нужно
  //   // звук еще может быть зацикленным, надо возвращать что то что мы могли бы использовать для удаления звука в очереди
  //   // если звук не статический относительно персонажа, то нам нужно сделать его моно
  //   // как понять статический или нет? вообще большинство звуков будет не статикой
  //   // музыка на заднем фоне + звуки меню - статика
  //   // + некоторое количество игровых звуков
  //   // например звуки передвижения персонажа... вряд ли
  //   // это такие игровые звуки которые не слышит никто кроме локального игрока
  // }
  //
  // while (true) {
  //   s.update(1000);
  //   std::this_thread::sleep_for(std::chrono::microseconds(1000));
  // }

  // spdlog::set_level(spdlog::level::trace);

  // {
  //   traced_func();
  // }

  // {
  //   const utils::tracer t12313;
  //   traced_func2();
  // }

  // {
  //   utils::tracer t;
  //   traced_func3();
  // }

  //utils_assertf(5==4, "test assert {}", 1);
  //utils_assert(5==4);
  //assert(5==4);

  utils::println(utils::type_name<decltype(&main)>(), utils::type_id<decltype(&main)>());

  
  // надо наверное добавить сюда размер, на всякий случай?
  uint8_t buf[1024];
  memset(buf, 0, 1024);
  utils::bitstream bs(buf);

  enum abc {
    e1,
    e2,
    e3,
    e4,
    e5,
    e6,
    count
  };

  constexpr size_t enum_bits = utils::count_significant(abc::count);

  // можно ли считать что это дело готово?
  bs.write(enum_bits, e2);
  bs.write(enum_bits, e4);
  bs.write(enum_bits, e6);
  assert(static_cast<abc>(bs.peek_at(0, enum_bits)) == abc::e2);
  assert(static_cast<abc>(bs.peek_at(enum_bits, enum_bits)) == abc::e4);
  assert(static_cast<abc>(bs.peek_at(enum_bits+enum_bits, enum_bits)) == abc::e6);
  utils::println("enum_bits", enum_bits, "bs.pos", bs.position());

  return 0;
}

// небольшие мысли по поводу архитектуры: открыл vcmi там походу полностью клиент-серверная архитектура даже для локальной игры
// было бы неплохо сделать примерно тоже самое, там полностью определены все (?) типы взаимодействия в игре
// client/Client.h - множество функций например: giveHeroArtifact или setMovePoints
// я так понимаю мне нужно будет сделать что то похожее, но в моем случае почти все взаимодействия
// будут сделаны через скрипты, нужно синхронизовать выполнение скрипта, вообще выполнение игры должно быть разделено на "тики"
// состояние игры в определенный тик на клиенте должно соответствовать состоянию игры в тот же тик на сервере
// возможно это не стейт-оф-зе-арт в 2023, но это хотя бы понятно как сделать
// как общается клиент и сервер - по идее через "пайп", то есть через буфер сырых данных, 
// клиент запоняет буфер, сервер потихоньку считывает данные, считает мир и отправляет обратно изменения
// в скриптах мы четко знаем какая функция просто считывает данные, а какая только их задает 
// нужно брать игровые данные за О(1), у нас будет как минимум 2 типа инпута:
// инпут с клавы (передвижение), запуск скрипта (что то еще?)
// инпут с клавы можно передвать и так, там данных немного и они достаточно очевидны
// ЛЮБОЙ скрипт оканчивается запуском функции эффектов (точнее нас интересуют только скрипты эффекты)
// и тут у нас два варианта: либо полностью ждать на клиенте пока у нас придет нужный снапшот из сервака
// либо пытаться предугадать что будет и составить снапшот клиенту, а потом когда с сервера придет 