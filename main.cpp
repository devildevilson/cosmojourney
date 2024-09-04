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

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/vec4.hpp"

#include "sound/system.h"
#include "utils/core.h"
#include "utils/type_traits.h"
#include "utils/bitstream.h"
#include "utils/string.h"
#include "utils/fileio.h"
#include "demiurg/system.h"
#include "demiurg/modules_listing.h"
#include "sound_resource.h"
#include "utils/dice.h"
#include "utils/named_serializer.h"
#include "utils/time.h"
#include "input/core.h"
#include "input/events.h"
#include "thread/lock.h"
#include "painter/vulkan_header.h"
#include "painter/system.h"
#include "painter/system_info.h"
#include "painter/auxiliary.h"
#include "painter/glsl_source_file.h"
#include "painter/shader_crafter.h"
#include "painter/buffer_resources.h"
#include "painter/shader_source_file.h"
#include "painter/container.h"
#include "painter/queue_stages.h"
#include "painter/render_pass_resources.h"
#include "painter/pipelines_resources.h"
#include "painter/swapchain_resources.h"
#include "painter/attachments_container.h"
#include "painter/framebuffer_resources.h"
#include "painter/layouting.h"
#include "painter/common_stages.h"
#include "painter/render_pass_stages.h"

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

static void err_handler(int32_t code, const char* desc) {
  utils::error("GLFW throw an error code {}: {}", code, desc);
}

int g_scancode = -1;

static void key_func(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto name = input::key_name_native(key, scancode);
  if (name.empty()) name = "no name";

  if (name == "E") g_scancode = scancode;

  input::events::update_key(scancode, action);

  const auto state = input::events::current_event_state(scancode);
  const auto state_name = input::event_state::to_string(state);

  if (action != 0) utils::info("Press '{}' '{}' {} {}", state_name, name, key, scancode);
}

struct vec4 { float r,g,b,a; };

struct test2_t {
  vec4 abc;
  vec4* arr;
};

int main(int argc, char const *argv[]) {
  // пока что sleep_until выглядит самым приятным вариантом среди всех
  // если вдруг сон опять напортачит и будет слишком долгим то следующие кадры не ждем
  // но и при этом это не spin_lock, а значит проц не будет занят на 100 процентов 
  // легко почтитать и довольно просто понять на сколько отстаем
  // наверное даже можно заменить им вообще все структуры
  //std::this_thread::sleep_until
  //thread::spin_sleep_until // идем практически идеально ровно, но съедаем 100% ядра
  //thread::light_spin_sleep_until // съедаем окол 100% ядра, но все равно идем как попало
  //std::this_thread::sleep_for // сложно правильно расчитать время сна, сон может быть черти какой, но это легкий сон
  //thread::spin_sleep_for // сложно правильно расчитать время сна, спим идеально ровно то что расчитали, но съедаем 100% ядра
  //thread::light_spin_sleep_for // теже беды, но еще и нет никаких гарантий что мы спим ровно то что расчитали

  // попробовать запуститься и нарисовать треугольник
  // что мне нужно? шейдеры нужны

  input::init i(&err_handler);

  demiurg::system dsys(utils::project_folder() + "folder1/");
  // НЕ УКАЗЫВАТЬ ПОСЛЕДНИЙ СЛЕШ (то есть не делать как папки)
  dsys.register_type<painter::shader_source_file>("shaders", "vert,frag");
  dsys.register_type<painter::glsl_source_file>("include", "glsl");

  dsys.load_default_modules();
  dsys.parse_resources();

  {
    std::vector<demiurg::resource_interface*> resources;
    resources.reserve(1000);
    dsys.find("shaders/", resources);
    for (const auto ptr : resources) {
      ptr->load({});
    }
  }

  painter::system psys;
  psys.reload_configs();
  auto gc = psys.create_main_container();
  auto layout = psys.create<painter::layouting>(gc->device, painter::layouting::create_info{1, 1, 1, 1, 1, 1});
  auto w = input::create_window(800, 600, "triangle"); // окно поди можно создать когда угодно ранее
  auto surf = painter::create_surface(gc->instance, w);
  psys.set_window_surface(surf);
  auto sch = psys.create<painter::simple_swapchain>(gc->device, gc->physical_device, surf, 2);
  auto conf = psys.get_attachments_config("default");
  auto ac = psys.create<painter::attachments_container>(gc->device, gc->buffer_allocator, sch, std::move(conf));
  auto rp_conf = psys.get_render_pass_config("default");
  auto rp = psys.create<painter::main_render_pass>(gc->device, rp_conf, ac);
  auto fc = psys.create<painter::simple_framebuffer>(gc->device, rp, ac, sch);
  auto pl_conf = psys.get_graphics_pipeline_config("default");
  auto pl = psys.create<painter::simple_graphics_pipeline>(gc->device, layout->pipeline_layout, gc->cache, &dsys);
  pl->init(rp->render_pass, 0, pl_conf, rp_conf->subpasses[0].attachments.size(), rp_conf->subpasses[0].attachments.data());
  auto t_buf = psys.create<painter::common_buffer>(gc->buffer_allocator, 5 * sizeof(glm::vec4), painter::usage::vertex, painter::reside::host);
  auto ptr = reinterpret_cast<glm::vec4*>(t_buf->mapped_data());
  ptr[0] = glm::vec4(0.0f, 1.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f))));
  ptr[1] = glm::vec4(0.5f, 0.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f))));
  ptr[2] = glm::vec4(1.0f, 1.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))));

  psys.recreate(800, 600);

  painter::do_command(gc->device, gc->transfer_command_pool, gc->graphics_queue, gc->transfer_fence, [sch] (VkCommandBuffer buf) {
    vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::CommandBuffer b(buf);
    b.begin(cbbi);

    for (uint32_t i = 0; i < sch->max_images; ++i) {
      auto img = sch->frame_storage(i);
      vk::ImageSubresourceRange isr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
      const auto &[bar, ss, ds] = painter::make_image_memory_barrier(img, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, isr);
      vk::CommandBuffer(buf).pipelineBarrier(ss, ds, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, bar);
    }

    b.end();
  });

  painter::vertex_draw_provider vdp{ 3, 1, 0, 0 };

  // первый рисовальщик фреймов
  {
    auto cmd_bar2 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal), uint32_t(vk::ImageLayout::ePresentSrcKHR));
    auto cmd_bar1 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::ePresentSrcKHR), uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal));
    auto cmd_draw = psys.create<painter::draw>(&vdp);
    auto cmd_pl = psys.create<painter::pipeline_view>(pl);
    auto cmd_vtx = psys.create<painter::bind_vertex_buffers>(0, std::vector{t_buf->buffer}, std::vector{size_t(0)});
    auto cmd_rp = psys.create<painter::render_pass_main>(gc->device, fc);
    auto cmd_qs = psys.create<painter::queue_main>(gc->device, gc->graphics_command_pool, gc->graphics_queue, std::initializer_list<VkSemaphore>{}, std::initializer_list<uint32_t>{});
    auto cmd_p = psys.create_frame_presenter<painter::queue_present>(gc->device, gc->presentation_queue, sch->get_swapchain(), sch, cmd_qs);
    cmd_vtx->set_next(cmd_pl); cmd_pl->set_next(cmd_draw); 
    cmd_bar1->set_next(cmd_rp); cmd_rp->set_childs(cmd_vtx); cmd_rp->set_next(cmd_bar2);
    cmd_qs->set_childs(cmd_bar1);
  }
  
  // второй рисовальщик фреймов
  {
    auto cmd_bar2 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal), uint32_t(vk::ImageLayout::ePresentSrcKHR));
    auto cmd_bar1 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::ePresentSrcKHR), uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal));
    auto cmd_draw = psys.create<painter::draw>(&vdp);
    auto cmd_pl = psys.create<painter::pipeline_view>(pl);
    auto cmd_vtx = psys.create<painter::bind_vertex_buffers>(0, std::vector{t_buf->buffer}, std::vector{size_t(0)});
    auto cmd_rp = psys.create<painter::render_pass_main>(gc->device, fc);
    auto cmd_qs = psys.create<painter::queue_main>(gc->device, gc->graphics_command_pool, gc->graphics_queue, std::initializer_list<VkSemaphore>{}, std::initializer_list<uint32_t>{});
    auto cmd_p = psys.create_frame_presenter<painter::queue_present>(gc->device, gc->presentation_queue, sch->get_swapchain(), sch, cmd_qs);
    cmd_vtx->set_next(cmd_pl); cmd_pl->set_next(cmd_draw);
    cmd_bar1->set_next(cmd_rp); cmd_rp->set_childs(cmd_vtx); cmd_rp->set_next(cmd_bar2);
    cmd_qs->set_childs(cmd_bar1);
  }

  const size_t target_fps = 1000000.0 / 500.0;

  // чет все глючит и фреймы мне не показывает почему?

  size_t frame_counter = 0;
  auto tp = std::chrono::high_resolution_clock::now();
  while (!input::should_close(w)) {
    frame_counter += 1;
    const auto next_tp = tp + std::chrono::microseconds(frame_counter * target_fps);

    const size_t one_second = 1000ull * 1000ull * 1000ull;
    const auto res = vk::Result(psys.wait_frame(one_second));
    if (res != vk::Result::eSuccess) utils::error("Wait for prev frame returned '{}'", vk::to_string(res));
    psys.compute_frame();

    std::this_thread::sleep_until(next_tp);
  }

  painter::destroy_surface(gc->instance, surf);
  return 0;
}

// вот такой есть способ алиаса, относительно неплохой
// к сожалению наверное не подойдет в качестве исходной функции для lua и скриптового биндинга
// единственный нормальный способ это в макрос списком новые имена передать увы (ебанина коня)
//template <typename... Args>
//auto g(Args &&...args) -> decltype(f(std::forward<Args>(args)...)) {
//  return f(std::forward<Args>(args)...);
//}

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

// вот тут какая мысля появилась - у нас И звуки И графика требуют той или иной интерполяции для таких данных
// как положение и направление движения и скорость и еще парочка
// (положение и скорость кстати нужны еще чтобы интерполировать отсутствие данных из сети)
// нам необходим некий один большой буффер с этими данными из которого будет браться как раз интерполяция
// как это сделать? так же на счет анимации - буфер который хранит итоговые матрицы может получиться просто гиганстким
// можно записывать анимационные текстуры (ну тоже в общем то гигансткий буфер)
// короче надо что то придумать с интерполяцией
// как будто после того как посчитается физика должен идти некий шаг где мы записываем новые положения в конечный буфер
// а потом куча других систем считают из этого буфера, то есть рид-врайт мьютекс

// похоже что можно сделать вот так
//std::shared_mutex write;  // use boost's or c++14
//
//// One write, no reads.
//void write_fun() {
//  std::lock_guard<std::shared_mutex> lock(write);
//  // DO WRITE
//}
//
//// Multiple reads, no write
//void read_fun() {
//  std::shared_lock<std::shared_mutex> lock(write);
//  // do read
//}