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
#include "demiurg/module_system.h"
#include "demiurg/resource_system.h"
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
#include "bindings/lua_header.h"
#include "visage/font.h"

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

  //input::init i(&err_handler);

  //demiurg::module_system msys(utils::project_folder() + "folder1/");
  //demiurg::resource_system dsys;
  //// НЕ УКАЗЫВАТЬ ПОСЛЕДНИЙ СЛЕШ (то есть не делать как папки)
  //dsys.register_type<painter::shader_source_file>("shaders", "vert,frag");
  //dsys.register_type<painter::glsl_source_file>("include", "glsl");

  //msys.load_default_modules();
  //dsys.parse_resources(&msys);

  // надо добавить в демиург поиск по типу отца

  // походу время пререйти к интерфейсу, интерфейс добавит 2 новых ресурсов
  // луа скрипты и шрифты + потом добавятся скинчики и стили
  // в чем прикол интерфейса? мы запустим функцию каждый кадр
  // которая опросит множество биндингов и построит интефрейс по биндингам наклира
  // биндинги наклира возможно придется писать самому
  // ну и как будто бы все, в чем сложность?
  // у нас есть несколько состояний игры в которой она находится по мере работы
  // некоторые вещи недоступны или выглядят чуть иначе в другом состоянии
  // решение? использовать разные системы для каждого типа?
  // в скором времени может превратиться к говно
  // использовать одну систему для всего?
  // есть шанс дурацких ошибок, хотя если я напишу эти вещи, то как будто не должно происходить
  // я скорее склоняюсь ко второму варианту
  // интерфейс должен быть тем или иным образом огражден от проблем
  // и скорее лучше получить nil чем залезть в память которая еще не готова
  // для интерфейса по большому счету нужны картинки и информация об игре
  // и вот информация об игре как раз может быть проблемой
  // тут придется придумать что нибудь
  // ну короч таким образом нужно сделать следущее:
  // 1) функции подготовки и обновления наклира на стороне сипипи
  // 2) обработку ресурсов: скрипты, картинки, шрифты и проч. какого размера должны быть шрифты?
  // 3) наверное было бы неплохо придумать умный стак для nk стилей
  // 4) биндинги для наклира такие что: они не должны раскрывать ненужные внутренности
  // типа создания шрифтов и инпута + синтаксический сахар для всяких прикольных штук
  // 5) биндинги для остальной игры с какими то мерами по защите
  // было бы неплохо назвать эту часть visage

  // интерфейс наверное пойдет в свой сабмит в котором сразу на картинку свопчейна
  // будем рисовать, а остальные вещи в отдельных аттачментах
  // а если мы хотим MSAA сделать, то как быть?
  // MSAA поди имеет смысл только в основной части рендеринга

  //dsys.load_default_modules();
  //dsys.parse_resources();

  //{
  //  std::vector<demiurg::resource_interface*> resources;
  //  resources.reserve(1000);
  //  dsys.find("shaders/", resources);
  //  for (const auto ptr : resources) {
  //    ptr->load({});
  //  }
  //}

  //painter::system psys;
  //psys.reload_configs();
  //psys.dump_configs_to_disk();
  //auto conf = psys.get_attachments_config("default");
  //auto rp_conf = psys.get_render_pass_config("default");
  //auto pl_conf = psys.get_graphics_pipeline_config("default");

  //auto gc     = psys.create_main_container();
  //auto layout = psys.create<painter::layouting>(gc->device, painter::layouting::create_info{1, 1, 1, 1, 1, 1});
  //auto w      = input::create_window(800, 600, "triangle"); // окно поди можно создать когда угодно ранее
  //auto surf   = painter::create_surface(gc->instance, w);
  //auto sch    = psys.create<painter::simple_swapchain>(gc->device, gc->physical_device, surf, 2);
  //auto scont  = psys.create<painter::surface_container>(gc->instance, surf);
  //auto ac     = psys.create<painter::attachments_container>(gc->device, gc->buffer_allocator, sch, std::move(conf));
  //auto rp     = psys.create<painter::main_render_pass>(gc->device, rp_conf, ac);
  //auto fc     = psys.create<painter::simple_framebuffer>(gc->device, rp, ac, sch);
  //auto pl     = psys.create<painter::simple_graphics_pipeline>(gc->device, layout->pipeline_layout, gc->cache, &dsys);
  //rp->create_render_pass();
  //pl->init(rp->render_pass, 0, pl_conf, rp_conf->subpasses[0].attachments.size(), rp_conf->subpasses[0].attachments.data());
  //auto t_buf = psys.create<painter::common_buffer>(gc->buffer_allocator, 5 * sizeof(glm::vec4), painter::usage::vertex, painter::reside::host);
  //auto ptr = reinterpret_cast<glm::vec4*>(t_buf->mapped_data());
  //ptr[0] = glm::vec4(0.0f, 1.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f))));
  //ptr[1] = glm::vec4(0.5f, 0.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f))));
  //ptr[2] = glm::vec4(1.0f, 1.0f, 0.0f, glm::uintBitsToFloat(glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))));
  //t_buf->flush_memory();

  //psys.recreate(800, 600);

  //painter::do_command(gc->device, gc->transfer_command_pool, gc->graphics_queue, gc->transfer_fence, [sch] (VkCommandBuffer buf) {
  //  for (uint32_t i = 0; i < sch->max_images; ++i) {
  //    auto img = sch->frame_storage(i);
  //    vk::ImageSubresourceRange isr(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
  //    const auto &[bar, ss, ds] = painter::make_image_memory_barrier(img, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, isr);
  //    vk::CommandBuffer(buf).pipelineBarrier(ss, ds, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, bar);
  //  }
  //});

  //painter::vertex_draw_provider vdp{ 3, 1, 0, 0 };

  //// первый рисовальщик фреймов
  //{
  //  auto cmd_bar2 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal), uint32_t(vk::ImageLayout::ePresentSrcKHR));
  //  auto cmd_bar1 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::ePresentSrcKHR), uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal));
  //  auto cmd_draw = psys.create<painter::draw>(&vdp);
  //  auto cmd_pl   = psys.create<painter::pipeline_view>(pl);
  //  auto cmd_vtx  = psys.create<painter::bind_vertex_buffers>(0, std::vector{t_buf->buffer}, std::vector{size_t(0)});
  //  auto cmd_rp   = psys.create<painter::render_pass_main>(gc->device, fc);
  //  auto cmd_qs   = psys.create<painter::queue_main>(gc->device, gc->graphics_command_pool, gc->graphics_queue, std::initializer_list<VkSemaphore>{}, std::initializer_list<uint32_t>{});
  //  auto cmd_p    = psys.create_frame_presenter<painter::queue_present>(gc->device, gc->presentation_queue, sch, sch, cmd_qs);
  //  cmd_pl->set_next(cmd_vtx); cmd_vtx->set_next(cmd_draw);
  //  cmd_rp->set_childs(cmd_pl); 
  //  //cmd_bar1->set_next(cmd_rp); cmd_rp->set_next(cmd_bar2);
  //  cmd_qs->set_childs(cmd_rp);
  //}
  //
  //// второй рисовальщик фреймов
  //{
  //  auto cmd_bar2 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal), uint32_t(vk::ImageLayout::ePresentSrcKHR));
  //  auto cmd_bar1 = psys.create<painter::change_frame_image_layout>(sch, uint32_t(vk::ImageLayout::ePresentSrcKHR), uint32_t(vk::ImageLayout::eShaderReadOnlyOptimal));
  //  auto cmd_draw = psys.create<painter::draw>(&vdp);
  //  auto cmd_pl   = psys.create<painter::pipeline_view>(pl);
  //  auto cmd_vtx  = psys.create<painter::bind_vertex_buffers>(0, std::vector{t_buf->buffer}, std::vector{size_t(0)});
  //  auto cmd_rp   = psys.create<painter::render_pass_main>(gc->device, fc);
  //  auto cmd_qs   = psys.create<painter::queue_main>(gc->device, gc->graphics_command_pool, gc->graphics_queue, std::initializer_list<VkSemaphore>{}, std::initializer_list<uint32_t>{});
  //  auto cmd_p    = psys.create_frame_presenter<painter::queue_present>(gc->device, gc->presentation_queue, sch, sch, cmd_qs);
  //  cmd_pl->set_next(cmd_vtx); cmd_vtx->set_next(cmd_draw);
  //  cmd_rp->set_childs(cmd_pl); 
  //  //cmd_bar1->set_next(cmd_rp); cmd_rp->set_next(cmd_bar2);
  //  cmd_qs->set_childs(cmd_rp);
  //}

  //// полезных стандартных лэйаутов на самом деле не очень много
  //// для них можно сделать мэп, другое дело нужно ли повторять один к одному?
  //// возможно имеет смысл сделать image_target

  //// наверное имеет смысл подключать VK_KHR_MAINTENANCE1 ?
  //// так можно перевернуть Y координату просто изменив настройки вьюпорта
  //// или использовать Vulkan 1.1

  //const size_t target_fps = 1000000.0 / 144.0;

  //// чет все глючит и фреймы мне не показывает почему?
  //// короче говоря 2 проблемы: 
  //// vk::CullModeFlagBits::eFrontAndBack ЭТО КУЛЛИНГ ОБЕИХ СТОРОН ТРЕУГОЛЬНКА
  //// А ЗНАЧИТ НИЧЕГО РИСВАТЬСЯ НЕ БУДЕТ
  //// vkPipelineMultisampleStateCreateInfo - указываем количество семплов в изображении
  //// и можем указать маски для шейдера, ЕСЛИ УКАЗЫВАЕМ ТО МАСКА ДОЛЖНА БЫТЬ НЕ НОЛЬ
  //// ЧТОБЫ ХОТЬ ЧТО ТО УВИДЕТЬ

  //size_t frame_counter = 0;
  //auto tp = std::chrono::high_resolution_clock::now();
  //while (!input::should_close(w)) {
  //  frame_counter += 1;
  //  const auto next_tp = tp + std::chrono::microseconds(frame_counter * target_fps);

  //  input::poll_events();

  //  const size_t one_second = 1000ull * 1000ull * 1000ull;
  //  const auto res = vk::Result(psys.wait_frame(one_second));
  //  if (res != vk::Result::eSuccess) utils::error("Wait for prev frame returned '{}'", vk::to_string(res));
  //  psys.compute_frame();

  //  std::this_thread::sleep_until(next_tp);
  //}

  //const size_t one_second = 1000ull * 1000ull * 1000ull;
  //const auto res = vk::Result(psys.wait_frame(one_second));
  //if (res != vk::Result::eSuccess) utils::error("Wait for prev frame returned '{}'", vk::to_string(res));

  // блен надо свопчеин раньше чем сюрфейс удалять... сюрфейс надо просто передать в класс свопчейна
  //painter::destroy_surface(gc->instance, surf);

  /*sol::state lua;
  lua.open_libraries(
    sol::lib::debug,
    sol::lib::base, 
    sol::lib::bit32, 
    sol::lib::coroutine, 
    sol::lib::math, 
    sol::lib::package, 
    sol::lib::string, 
    sol::lib::table, 
    sol::lib::utf8
  );

  const auto stack_print = [] (sol::this_state s) {
    lua_Debug info;
    int level = 0;
    while (lua_getstack(s.L, level, &info)) {
        lua_getinfo(s.L, "nSl", &info);
        fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n",
            level, info.short_src, info.currentline,
            (info.name ? info.name : "<unknown>"), info.what);
        ++level;
    }
  };*/

  /*const auto lam = [] (sol::this_state s) {
    char buffer_name[512]{0};
    char buffer[4096]{0};
    char buffer2[20]{0};
    char buffer3[20]{0};
    lua_Debug ar;
    memset(&ar, 0, sizeof(ar));
    ar.source = buffer;
    ar.name = buffer_name;
    ar.namewhat = buffer2;
    ar.what = buffer3;
    int32_t ret;
    size_t len = 0;
    std::string src;
    ret = lua_getstack(s.L, 0, &ar);
    lua_getinfo(s.L, ">Sl", &ar);
    len = strlen(ar.name);
    src = std::string(ar.source, ar.srclen);
    utils::println(0, len == 0 ? "(no name)" : ar.name, ar.currentline);
    ret = lua_getstack(s.L, 1, &ar);
    lua_getinfo(s.L, ">Sl", &ar);
    len = strlen(ar.name);
    src = std::string(ar.source, ar.srclen);
    utils::println(1, len == 0 ? "(no name)" : ar.name, ar.currentline);
    ret = lua_getstack(s.L, 2, &ar);
    lua_getinfo(s.L, ">Sl", &ar);
    len = strlen(ar.name);
    src = std::string(ar.source, ar.srclen);
    utils::println(2, len == 0 ? "(no name)" : ar.name, ar.currentline);
  };*/

  //const auto lam = [] (sol::this_state s) {
  //  lua_Debug info;
  //  lua_getstack(s.L, 1, &info);
  //  lua_getinfo(s.L, "nSl", &info);
  //  auto src = info.source; // общая строка функции которая вызывает
  //  auto name = info.name; // кто вызывает
  //  auto line = info.currentline; // где вызывается

  //  // тут мы еще можем взять как называется текущая функция, но ее уже сложно будет "вмешать в хеш"
  //  utils::println(std::string(src, info.srclen), name != nullptr ? name : "<no name>", line);
  //};

  //const auto inc = [] (sol::object o) {
  //  int x = o.as<int>();
  //  x+=1;
  //  //o = sol::make_object(o.lua_state(), x);
  //  //o.
  //};
  
  /*lua.set_function("lambda1", lam);
  lua.set_function("lambda2", lam);
  lua.set_function("lam2353", lam);
  lua.set_function("stack_print", stack_print);*/
  //lua.set_function("inc", inc);

  //lua.script("function abc() \nstack_print(); \nend; \nfunction abcabc() \nabc(); \nstack_print(); \nend; \nabcabc()");
  //lua.script("function abc() \nlam2353(); \nend; \nfunction abcabc() \nabc(); \nlambda2(); \nend; \nabcabc()");
  //lua.script("abcabc()");
  //lua.script("local x = 0; print(x); inc(x); print(x);");

  // координаты тут приходят с перевернутой Y координатой
  // но пока что как сопоставить с Наклир фонтом я так и не понял

  // где g.x,g.y,g.w,g.h - позиция X, позиция перевернутый Y, размер W, размер H
  // g.al,g.ab,g.ar,g.at - MIN X, MIN Y, MAX X, MAX Y (бокс)
  // g.pl,g.pb,g.pr,g.pt - не понятно
  // xadvance - непонятно правильный или нет, он в каких то долях приходит?
  // для наклира я так понимаю нужен бокс + UV по боксу + размеры отдельно
  // короче я понял что такое плэйн: это то как расположен глиф
  // относительно "бокса в котором он рендерился" (то есть сейчас по умолчанию взят размер 24 setMinimumScale)
  // 

  // мне нужно заполнить вот это nk_user_font_glyph из данных ниже
  // тут скорее всего uv понятно откуда брать (размер на атласе / размер фонта)
  // offset - это отступ от левого верхнего угла до глифа (то есть значения plane с обратным Y)
  // width, height - размер глифа (атлас макс - атлас мин, а ну или просто размер в инте)
  // xadvance - это отступ до следущего глифа
  //struct nk_user_font_glyph {
  //  struct nk_vec2 uv[2];
  //  /* texture coordinates */
  //  struct nk_vec2 offset;
  //  /* offset between top left and glyph */
  //  float width, height;
  //  /* size of the glyph  */
  //  float xadvance;
  //  /* offset to the next glyph */
  //};
  // похоже что НК считает все глифики слева сверху
  // а тут они записаны снизу слева и надо бы перевычислить значения

  const auto f = utils::perf("load_font", visage::load_font, utils::project_folder() + "font.ttf");
  utils::println(f->width, f->height, f->scale, f->metrics.em_size, f->metrics.line_height);
  for (const auto &g : f->glyphs2) {
    // примерно вот так
    const int ny = f->height - (g.y + g.h);
    const double nab = double(f->height) - g.at;
    const double nat = double(ny + g.h) - 0.5;
    const double npb = 1.0 - g.pt;
    const double npt = 1.0 - g.pb;

    // расчет данных для НК глифа
    double uvminx = g.al / double(f->width);
    double uvminy =  nab / double(f->height);
    double uvmaxx = g.ar / double(f->width);
    double uvmaxy =  nat / double(f->height);

    double offsetx = g.pl * f->scale;
    double offsety =  npb * f->scale;

    // сомневаюсь что я должен брать эти значения
    // точнее я поди должен брать полный размер и вычитать субпиксель?
    // .... возможно, видимо выясню только отрендерив картинку
    //double width  = (g.pr - g.pl) * f->scale;
    //double height = (npt - npb) * f->scale;
    // или так?
    double width  = g.w;
    double height = g.h;

    double xadvance = g.xadvance * f->scale;

    utils::println("codepoint:", g.codepoint);
    utils::println("xadvance:", g.xadvance, "advances:", f->scale*g.xadvance, "scale:", g.scale, "gscale:", g.gscale);
    utils::println("rect  :",g.x,ny,g.w,g.h);
    utils::println("atlas :",g.al,nab,g.ar,nat);
    utils::println("plane :",g.pl,npb,g.pr,npt);
    //utils::println("planex:",f->scale*g.pl,f->scale*g.pb,f->scale*g.pr,f->scale*g.pt);
    utils::println("uv    :",uvminx,uvminy,uvmaxx,uvmaxy);
    utils::println("offset:",offsetx,offsety);
    utils::println("size  :",width,height);
  }

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
