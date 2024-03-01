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

  std::unique_ptr<sound::system::resource> res1;
  std::unique_ptr<sound::system::resource> res2;
  std::unique_ptr<sound::system::resource> res3;

  {
    const std::string file_name = "phonk.mp3";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res1 = std::make_unique<sound::system::resource>("test_mp3", sound::system::resource::type::mp3, std::move(buffer));
  }

  {
    const std::string file_name = "ferambie.ogg";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res2 = std::make_unique<sound::system::resource>("test_ogg", sound::system::resource::type::ogg, std::move(buffer));
  }

  {
    const std::string file_name = "save.wav";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res3 = std::make_unique<sound::system::resource>("test_wav", sound::system::resource::type::wav, std::move(buffer));
  }

  sound::system s;
  
  size_t id = 0;
  {
    utils::time_log log("start playing"); // чет долго ужс, надо первую загрузку спихнуть в апдейт
    id = s.setup_sound(res1.get(), sound::settings());
  }
  
  while (true) {
    s.update(1000);
    const float abc = s.stat_sound(id);
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }

  // spdlog::set_level(spdlog::level::trace);

  // {
  //   traced_func();
  // }

  // {
  //   //const utils::tracer t12313;
  //   traced_func2();
  // }

  // {
  //   utils::tracer t;
  //   traced_func3();
  // }

  ////utils_assertf(5==4, "test assert {}", 1);
  ////utils_assert(5==4);
  ////assert(5==4);

  //utils::println(utils::type_name<decltype(&main)>(), utils::type_id<decltype(&main)>());

  
  // надо наверное добавить сюда размер, на всякий случай?
  //uint8_t buf[1024];
  //memset(buf, 0, 1024);
  //utils::bitstream bs(buf);

  //enum abc {
  //  e1,
  //  e2,
  //  e3,
  //  e4,
  //  e5,
  //  e6,
  //  count
  //};

  //constexpr size_t enum_bits = utils::count_significant(abc::count);

  //// можно ли считать что это дело готово?
  //bs.write(enum_bits, e2);
  //bs.write(enum_bits, e4);
  //bs.write(enum_bits, e6);
  //assert(static_cast<abc>(bs.peek_at(0, enum_bits)) == abc::e2);
  //assert(static_cast<abc>(bs.peek_at(enum_bits, enum_bits)) == abc::e4);
  //assert(static_cast<abc>(bs.peek_at(enum_bits+enum_bits, enum_bits)) == abc::e6);
  //utils::println("enum_bits", enum_bits, "bs.pos", bs.position());

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

// обновление по написанному - к сожалению нам не удастся никак сократить выполнение скрипта
// его обязательно нужно выполнять последовательно с изменениями данных
// поэтому в снапшоте имеет смысл хранить инфу как то вот так
// use script script1:
// [changes list]
// где мы поймем когда и кто запустил скрипт и что после него поменялось
// если изменения не совпадают с ожидаемыми, то у нас десинхрон
// + к этому наконец то нашел хороший способ справится со всеми бедами 
// времени в игре: нужно распространить в этот проект следующее
// будет минимум 3 потока: 
// поток игры (скрипты, физика) - конкретные тики синхронизированные с сервером
// поток графики - 144 фпс например с интерполяцией
// поток звуков - 6 фпс подгружаем потихоньку звуки в игру
// к сожалению теперь не очень понятно что делать с threadpool
// сокращать количество потоков там на 3? или предполагаем что звуки успеют отработать без проблем

// здесь надо бы создать в каком то виде классы обертки над основными системами (игра, звук, графика)
// по идее игра состоит из последовательного запуска систем которые работают с энтити
// значит условно у нас будет несколько запусков систем, а затем система которая закроет мьютекс на графике например
// выгрузит туда данные и снова его откроет, ну в общем то как будто нужно
// выделить компонент который примет данные, а потом по нему все скопом обновится в графике или звуке
// в общем надо придумать как сделать тайловую графику похожую на то что было в старбаунде:
// у тайла скорее всего несколько слоев: нижний - чисто текстурка земли, верхний - рамка (например трава на земле)
// возможно это один слой, просто сделаны 9 вариантов тайла + тайл может получить какой то особый рисунок (ржавчина кое где и проч)
// наверное было бы класно для каждого тайла прикрутить уникальное число, изменяя которое можно изменять рисунок
// то есть нажатие кнопки по тайлу поменяет число на следующее и тайл поменяет текстурку
// это число изначально вычисляется случайно на основе координат
// тайл переднего плана + тайл заднего + динамический фон

// тайлов на планете похоже что будет очень много, надо во первых сделать тайл максимально упакованным
// а во вторых придумать как эффективно в них ориентироваться

// что то вроде этого
struct gpu_tile {
  uint32_t x, y; // как то нужно вычислить расположение на экране причем желательно чтобы итоговые координаты были маленькими
  uint32_t unique_num; // число возможно менятся не будет (в большинстве случаев)
  uint32_t type; // находим текстурки по этому типу
};

// до этого проекта имеет смысл сделать что то еще мельче 
// например вариацию на тему vampiric survivers 
// в чем суть: тайловая карта с простыми формами по которой движется спрайт игрока 
// вокруг постоянно появляются противники которые движутся в сторону игрока
// большая часть из них наносит урон при прикосновении + некоторые спавнят снаряды
// очень простой ИИ который скорее всего даже не реагирует на препятствия
// суть игры продержаться против волн врагов как можно дольше
// таким образом на карте у нас есть: 
// 1) тайлы земли - подойдет какой нибудь тайловый движок в зачаточном виде
// 2) объекты и декорации - вообще возможно не будут занимать какой то конкретный тайл 
// а скорее иметь x,y координату
// 3) предметы, монстры и игрок - энтити с логикой
// 4) разные спецэффекты 
// было бы неплохо придумать механизм бесконечной карты - 
// доступно движение в любую сторону на любое расстояние без деградации точности float
// от игрока по сути требуется только выбирать направление атаки + собственно двигаться
// концепт простой, на нем можно отточить как раз вещи которые например описаны выше

// желательно к этому делу как раз попробовать сделать свой интерфейс
// возможно в расках проекта можно немного покрутить нетворкинг

// с чего начать? нужно сначала сюда прикрутить рендеринг и попытаться порисовать базовые формы

// надо чутка подумать какой интерфейс подойдет для системы звуков и наверное сверху сделать обертку
// которая запустит звуки в отдельном потоке