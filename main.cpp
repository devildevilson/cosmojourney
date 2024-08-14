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
#include "utils/string.h"
#include "demiurg/system.h"
#include "demiurg/modules_listing.h"
#include "sound_resource.h"
#include "utils/dice.h"
#include "utils/named_serializer.h"
#include "utils/time.h"

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
  std::ifstream file(file_name, std::ios::binary);
  if (!file) throw std::runtime_error("Could not load file " + file_name);
  //if (!file.is_open()) throw std::runtime_error("Could not load file " + file_name);

  // file.seekg(0,std::ios::end);
  // const size_t size = file.tellg();
  // file.seekg(0, std::ios::beg);
  //
  // file.read(&buffer[0], size);
  buffer = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  //utils::println("File", file_name, "size", buffer.size());
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

void parse_table_line(
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

struct def {
  double g;
  double h;
  double i;
};

struct abc {
  int a;
  float b;
  double c;
  std::string d;
  std::string_view e;
  //const char* f;
  struct def def;
  std::vector<int> vector;
  std::vector<std::string> vector_s;
  std::array<int, 2> arr;
  std::unordered_map<std::string_view, int> map;
};
//BOOST_DESCRIBE_STRUCT(abc, (), (a, b, c, d, e, f))

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

  /*std::unique_ptr<sound::system::resource> res1;
  std::unique_ptr<sound::system::resource> res2;
  std::unique_ptr<sound::system::resource> res3;
  std::unique_ptr<sound::system::resource> res4;

  {
    const std::string file_name = "SARIGAMI.mp3";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res1 = std::make_unique<sound::system::resource>("test_mp3", sound::system::resource::type::mp3, std::move(buffer));
    utils::println("File", file_name, "duration:", res1->duration(), "s");
  }

  {
    const std::string file_name = "swim2.ogg";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res2 = std::make_unique<sound::system::resource>("test_ogg", sound::system::resource::type::ogg, std::move(buffer));
    utils::println("File", file_name, "duration:", res2->duration(), "s");
  }*/

  /*{
    const std::string file_name = "piano.wav";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res3 = std::make_unique<sound::system::resource>("test_wav", sound::system::resource::type::wav, std::move(buffer));
    utils::println("File", file_name, "duration:", res3->duration(), "s");
  }*/

  /*{
    const std::string file_name = "senbonzakura.flac";
    utils::time_log log("loading resource " + file_name);
    std::vector<char> buffer;
    load_file(file_name, buffer);
    res4 = std::make_unique<sound::system::resource>("test_flac", sound::system::resource::type::flac, std::move(buffer));
    utils::println("File", file_name, "duration:", res4->duration(), "s");
  }*/

  //sound::system s;
  //
  //size_t id = 0;
  //{
  //  utils::time_log log("start playing");
  //  sound::settings ss;
  //  ss.is_loop = true;
  //  id = s.setup_sound(res1.get(), ss);
  //}

  //s.set_master_volume(0.1f);
  //
  //float master_gain = 0.0f;
  //size_t counter = 0;
  //const double dur = res1->duration();
  //while (true) {
  //  s.update(100000);
  //  const double pos = s.stat_sound(id);
  //  const double cur_seconds = pos * dur;
  //  utils::println("Stat", pos, ":", cur_seconds, "s (", dur, "s) master_gain", master_gain);

  //  //if (abc > 0.05 && abc < 0.5) s.set_sound(id, 0.5);
  //  //counter += 1;
  //  //if (counter%30 == 0) master_gain += 0.05f;

  //  //s.set_master_volume(master_gain);

  //  std::this_thread::sleep_for(std::chrono::microseconds(100000));
  //}

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

  //const size_t arr_size = 3;
  //std::array<std::string_view, arr_size> arr;
  //const std::string_view test = "abc+def";
  //const size_t count = utils::string::split(test, "+", arr.data(), arr_size);
  //utils::println("count", count);
  ////assert(count == SIZE_MAX);
  //for (size_t i = 0; i < std::min(count, arr_size); ++i) {
  //  utils::print(i+1, arr[i], ";");
  //}
  //utils::println();

  //const std::string_view test2 = "event [ guard1, guard2 ]";
  //const auto inside = utils::string::trim(utils::string::inside(test2, "[", "]"));
  //utils::println(test2, "|", inside);

  //const std::string_view test3 = "   aa  a   ";
  //utils::println(utils::string::trim(test3));

  //const std::string_view table_line = "*s1 + e1 [ guard1, guard2 ] -> core/states/scripts/action1, core/states/scripts/action2 = s2";
  //std::string_view current_state, final_state, event;
  //std::array<std::string_view, 8> guards, actions;
  //parse_table_line(table_line, current_state, event, guards.data(), 8, actions.data(), 8, final_state);
  //utils::println(table_line);
  //utils::print(current_state, "+", event, "[", guards[0]);
  //for (size_t i = 1; guards[i] != ""; ++i) {
  //  utils::print(",", guards[i]);
  //}
  //utils::print(" ] ->", actions[0]);
  //for (size_t i = 1; actions[i] != ""; ++i) {
  //  utils::print(",", actions[i]);
  //}
  //utils::print(" =", final_state);
  //utils::println();

  //demiurg::system sys("folder1");
  //sys.register_type<cosmojourney::sound_resource>("sound", "mp3,ogg,flac,wav");
  //// в первый раз оказалось дольше
  //{
  //  utils::time_log l("parse_file_tree");
  //  sys.parse_file_tree();
  //}

  //{
  //  utils::time_log l("parse_file_tree");
  //  sys.parse_file_tree();
  //}

  //{
  //  utils::time_log l("sound/ferambie");
  //  const auto found1 = sys.find("sound/ferambie");
  //  utils::println();
  //  utils::println("sound/ferambie", found1.size());
  //  for (const auto ptr : found1) {
  //    utils::println(ptr->id);
  //  }
  //}

  //{
  //  utils::time_log l("sound/s");
  //  const auto found2 = sys.find("sound/s");
  //  utils::println();
  //  utils::println("sound/s", found2.size());
  //  for (const auto ptr : found2) {
  //    utils::println(ptr->id);
  //  }
  //}

  //{
  //  utils::time_log l("sound/");
  //  const auto found3 = sys.find("sound/");
  //  utils::println();
  //  utils::println("sound/", found3.size());
  //  for (const auto ptr : found3) {
  //    //utils::println(ptr->id);
  //    //ptr->loading(utils::safe_handle_t());
  //    for (auto rep = ptr; rep != nullptr; rep = rep->replacement_next(ptr)) {
  //      for (auto sup = rep; sup != nullptr; sup = sup->supplementary_next(rep)) {
  //        utils::println(sup->module_name, sup->id, sup->ext);
  //      }
  //    }
  //  }
  //}

  // приведение работает вот так
  //utils::safe_handle_t handle;
  //auto abc = (size_t *)handle;

  //utils::time_log l("prng");
  //auto state = utils::xoroshiro128starstar::init(123);
  //const size_t ret = utils::dice_accumulator(3, 20, state); // 3d20
  //utils::println("ret", ret);

  
  // glaze не работает с const char* !!!
  // ну и скорее всего вообще ни с какими указателями
  //abc s{ 1, 0.5f, 0.24, "string", "view", 
  //"char*", 
  //def{ 3.14, 5, 12 }, {1, 2, 3}, {"abc", "def", "asfaf"}, {4,5}, { { "ab", 45 }, { "de", 64 } } };

  //std::string c;
  //std::vector<uint8_t> v;
  //c.reserve(10000);
  //utils::to_binary(s, v);
  //utils::println(v.size(), v.data());
  //c.clear();
  //utils::to_lua(s, c);
  //utils::println(c);

  //utils::println(s);
  
  const std::string modules_root_path = "./folder1/";
  demiurg::modules_listing ml(modules_root_path);
  ml.reload();

  std::vector<std::string> paths;
  for (const auto &m : ml.entries()) {
    utils::println(m->path, m->hash, m->file_date);
    paths.push_back(m->path);
  }

  ml.save_list("list123", paths);

  // так что теперь
  demiurg::system s(modules_root_path);
  s.set_modules_list("list123");
  s.load_default_modules();

  // можно сделать настройки, что в настройках у нас есть?
  // по большому счету это структура + метод сериализации
  // 

  //const auto t = utils::timestamp();
  //const auto ms = s.get_modules();
  ////for (const auto &m : ms) {
  ////  utils::println(m.path, m.hash, t, m.timestamp, m.file_date);
  ////}

  //// записывать можно только путь, хеш сумму и метку времени
  //// а для интерфейса потребуется все данные
  //// я бы даже сказал что нужно создать ряд структур для файлов + представление для них в листах
  //// да как будто система примет в себя только лист с модулями
  //// а какая то внешняя система будет отвечать за менеджмент листов 
  //// звучит норм вообще то говоря
  //s.save_list("new1", ms);
  //const auto list = s.load_list("new1");
  //for (const auto &m : list) {
  //  utils::println(m.path, m.hash, m.file_date);
  //}

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
// в интерфейсе (наклир) похоже что только один недостаток: не умеет в дистанс филд фонтс
// в остальном может быть не совсем удобный интефрейс, возможно было бы неплохо сделать примерно как в ведроиде
// но не думаю что имеет смысл все это дело переделывать только по этим причинам
// надо наклир только покрасивше сделать

// с чего начать? нужно сначала сюда прикрутить рендеринг и попытаться порисовать базовые формы

// надо чутка подумать какой интерфейс подойдет для системы звуков и наверное сверху сделать обертку
// которая запустит звуки в отдельном потоке
