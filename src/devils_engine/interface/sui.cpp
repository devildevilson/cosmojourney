#include "sui.h"

namespace devils_engine {
  namespace sui {
    using namespace sui::literals;

    float unit::get(const context* ctx) const noexcept {
      const float mult = ctx->unit.mults[static_cast<size_t>(type)];
      return get(mult);
    }

    abs_rect compute_absolute(const context* ctx, const rect &r) {
      auto cur_window = ctx->current;
      utils_assertc(cur_window != nullptr, "probably forgot to create a window");
      auto prev_widget = cur_window->last;
      utils_assertc(prev_widget != nullptr, "current widget not found");

      // надо вынести в отдельную функцию где мы вернем посчитанный контент баундс
      float x = r.offset.x.get(ctx);
      float y = r.offset.y.get(ctx);
      float w = r.extent.w.get(ctx);
      float h = r.extent.h.get(ctx);
      if (r.offset.x.type == unit::type::relative) {
        x = x * prev_widget->layout->bounds.extent.x;
      } else if (r.offset.x.type == unit::type::not_specified) {
        x = 0;
      }

      if (r.offset.y.type == unit::type::relative) {
        y = y * prev_widget->layout->bounds.extent.y;
      } else if (r.offset.y.type == unit::type::not_specified) {
        y = 0;
      }

      if (r.extent.w.type == unit::type::relative) {
        w = w * prev_widget->layout->bounds.extent.x;
      } else if (r.extent.w.type == unit::type::not_specified) {
        w = prev_widget->layout->bounds.extent.x;
      }

      if (r.extent.h.type == unit::type::relative) {
        h = h * prev_widget->layout->bounds.extent.y;
      } else if (r.extent.h.type == unit::type::not_specified) {
        h = prev_widget->layout->bounds.extent.y;
      }

      return abs_rect(
        vec2(
          std::floor(x),
          std::floor(y)
        ), vec2(
          std::floor(w),
          std::floor(h)
        )
      );
    }

    raii_elem window(context* ctx, const std::string_view &name, const rect &bounds, const relative_to content) {
      // значит что мы тут должны сделать? добавить окно в стак, посмотреть было ли у нас окно ранее,
      // возможно поменять настройки,

      // найдем окно или создадим его:
      // как мы его храним? окон по идее не будет очень много (ну точнее недолжно быть много)
      // мы можем создать его в пуле окон и добавить в начало массива (или в начало листа)

      //utils::assert(ctx != nullptr, "abc");

      std::string_view text = "Обычная строка текста на русском языке";
      size_t index = 0;
      while (index < text.size()) {
        uint32_t rune = 0;
        const size_t count = utf_decode(text, rune);
        index += count;

        // находим эту руну в шрифте, смотрим на ее размер, получаем количество символов и ширину строки
      }

      const size_t hash = 1;
      auto w = ctx->find_window(hash);
      if (w == nullptr) {
        // создаем
      }

      auto layout = ctx->window_data.create<layout_default>(bounds, relative_to::top_left);
      w->last = ctx->window_data.create<widget_data_t>(nullptr, layout);

      // как хранить данные окна? окно не удаляется если мы его не видим по идее
      // значит мы там храним все старые данные
      // хотя наверное все равно надо бы как то понять что окно нужно подчистить
      // по идее нужно каждый раз вызывать эту функцию для того чтобы окно существовало
      //ptr->layout.bounds = bounds;

      // окно может быть закрыто, тогда трогать его ни к чему

      // где расчитывается extent? при запуске бокса или при разрушении elem?
      const auto elem = box(ctx, extent(300_dp, 1.0_frac));

      return raii_elem(ctx);
    }

    raii_elem box(context* ctx, const abs_rect &bounds, const relative_to content) {
      // в текущее окно нужно добавить бокс, когда считаем размеры?
      // насколько это вообще необходимо считать размеры бокса по детям? это скорее удобно
      //

      // где расчитывается extent? при запуске бокса или при разрушении elem?
      // по идее если отказаться от возможности сделать бокс размеры которого определяются его контентом
      // то мы резко сможем вычислять все размеры прямо на месте,
      // но вообще размеры которые не указаны мы можем просто пропускать, нам в любом случае нужно посчитать
      // размеры контента для всех боксов, позиции для боксов должны нам быть известны с самого начала
      // позиции для боксов берем по предыдущему боксу И relative_to
      // есть хорошая и очень маленькая библиотечка https://github.com/rxi/microui
      // какие то вещи можно подсмотреть там, там нет поддержки юникода
      // короч мне нужно скорее считать размер контента потом
      // нам может потребоваться добавить какой то контент для бокса у которого разные размеры
      // + нужно полосы прокрутки как то сделать, полосы прокрутки как раз основываются на размере контента
      // который мы вычисляем гораздо позже... полосы прокрутки в следующем кадре?
      // ну по крайней мере мы можем запомнить размеры контента для бокса после энда
      // хорошо, что делать с отрисовкой границ? тоже самое? нужно запомнить размер контента
      // из предыдущего кадра и это решит кучу проблем
      // что делать для боксов без хеша? предполагать что обычные настройки - это размер по размеру родителя

      // вообще если есть не дефолтный лайаут, то по идее он нам даст размеры... хотя не всегда
      // например мы хотим сделать несколько колонок колонки вниз могут быть очень большими
      // тут нам нужно использовать прокрутку, кстати о прокрутке ее тоже нужно учитывать при составлении вершин
      // к отрисовке, точнее при расчете абсолютной позиции виджета
      auto cur_window = ctx->current;
      utils_assertc(cur_window != nullptr, "probably forgot to create a window");
      auto prev_widget = cur_window->last;
      utils_assertc(prev_widget != nullptr, "current widget not found");

      // предыдущий - это клип для текущего, точнее клип потихоньку уменьшается походу продвижения в глубь интерфейса
      const auto pos = prev_widget->layout->compute_next(bounds);
      //const bool contain = prev_widget->layout->bounds.contain(pos);

      // было бы неплохо контроллировать общий аллокатор для всей гуишки
      //auto cur = cur_window->create_new_widget(pos, content, default_layout);
      auto l = ctx->window_data.create<layout_default>(pos, content);
      prev_widget = ctx->window_data.create<widget_data_t>(prev_widget, l);
      //const auto scissor = pos.shrink_to_intersection(r);

      // что тут? вот мы создали бокс и теперь следующие виджеты или контент
      // займут эту область

      return raii_elem(ctx);
    }

    raii_elem box(context* ctx, const rect &bounds, const relative_to content) {
      return box(ctx, compute_absolute(box, bounds), content);
    }

    void end(context* ctx) {
      // тут мы по идее у текущего окна должны "закрыть" предыдущий виджет
      // или закрыть само окно и положить команду к отрисовке?
      // в наклире в энде автор дорисовывал окно - не лучшая идея на мой взгляд
      // что такое закрыть с моей точки зрения? в энде мы имеем полное представление о том
      // сколько места у нас занимает контент и соответственно можем нарисовать виджет
      // виджет должен рисоваться по мере вызова функций
      //

      auto cur_window = ctx->current;
      utils_assertc(cur_window != nullptr, "probably forgot to create a window");
      auto cur_widget = cur_window->last;
      utils_assertc(cur_widget != nullptr, "current widget not found");

      cur_window->last = cur_widget->prev;
      auto layout = cur_widget->layout;
      ctx->window_data.destroy(cur_widget);
      ctx->window_data.destroy(layout);

      // убираем окно
      if (cur_window->last == nullptr) {
        ctx->current = nullptr;
      }
    }

    void fill(context* ctx, const color &c) {
      const auto r = ctx->current->last->layout->bounds;
      // по идее заливка идет по размерам в лэйауте
      // значит нужно создать простую команду
      // тип:
      struct color_rect_command {
        struct color color;
        struct rect rect;
      };

      // правильно команды мы задаем в окне, чтобы потом можно было нарисовать одно окно поверх другого
      // для хранения окон поди нужно использовать динамический массив, memmove не особо долгий в этом случае
      // зато отсортировать легко... не, можно листом сделать, достаточно в каждую функцию которая манипулирует
      // активностью окна добавить перемещение окна вперед
    }

    // функция должна быть очень простой: получаем бокс в котором располагается текст,
    // этот бокс может быть не виден из-за ограничителей, убираем то что не видно,
    // выделяем память на text_command и размер текста, задаем собственно текст, 
    // позицию, цвет, шрифт
    // как получать бокс? вообще я думал просто взять бокс последнего лэйаута
    // надо вот что: разделить собственно виджет текст и отрисовку текста
    // виджет будет брать новый бокс, а сейчас мне нужно сделать текст с чем есть
    // цвет и шрифт вынесем в контекст
    void text(context* ctx, const abs_rect &size, const std::string_view &text) {
      // просто текст по идее должен брать размер извне
      const size_t text_size = utils::align_to(text.size()+1, alignof(command_text));
      auto ptr = ctx->command_allocator.allocate(sizeof(command_text)+text_size);
      auto text_ptr = reinterpret_cast<char*>(ptr)+sizeof(command_text);
      memcpy(text_ptr, text.data(), text.size());
      text_ptr[text.size()] = '\0';
      auto comm_ptr = new (ptr) command_text(ctx->font, size, std::string_view(text_ptr, text.size()));
      if (ctx->commands->tail == nullptr) ctx->commands->head = ctx->commands->tail = comm_ptr;
      else utils::forw::list_add<0>(ctx->commands->tail, comm_ptr);
    }

    // layout_t::layout_t(const rect &bounds, const relative_to content, layout_func_t func) noexcept :
    //   bounds(bounds), counter(0), maximum_child_size(0,0), content(content), weights_summ(0.0f), inc_counter_and_get(std::move(func))
    // {}

    // layout_t::layout_t(const rect &bounds, const std::span<float> &weights, layout_func_t func) noexcept :
    //   bounds(bounds), counter(0), maximum_child_size(0,0),
    //   content(relative_to::top_left), weights_summ(0.0f), weights(weights),
    //   inc_counter_and_get(std::move(func))
    // {}

    // layout_t::layout_t(const rect &bounds, const rect &inner_bounds, layout_func_t func) noexcept :
    //   bounds(bounds), counter(0), maximum_child_size(0,0),
    //   inner_bounds(inner_bounds), content(relative_to::top_left), weights_summ(0.0f),
    //   inc_counter_and_get(std::move(func))
    // {}

    // widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept :
    //   layout(bounds, content, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
    // {
    //   // вообще нужно тут input_state посчитать
    // }

    // widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept :
    //   layout(bounds, weights, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
    // {}

    // widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept :
    //   layout(bounds, inner_bounds, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
    // {}

    widget_data_t(widget_data_t* prev, layout_i* layout) noexcept :
      layout(layout), input_state(0), prev(prev)
    {}

    window_t::window_t(const std::string_view &str) noexcept :
      hash(utils::murmur_hash64A(str, utils::default_murmur_seed)),
      name_size(str.size()),
      widget_stack(sizeof(widget_data_t)*100, sizeof(widget_data_t), alignof(widget_data_t)),
      last(nullptr),
      parent(nullptr)
    {
      assert(str.size() < maximum_window_name_size);
      memset(name_str, 0, maximum_window_name_size * sizeof(char));
      memcpy(name_str, str.data(), str.size());
    }

    // widget_data_t* window_t::create_new_widget(const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept {
    //   last = widget_stack.create<widget_data_t>(last, bounds, content, std::move(func));
    //   return last;
    // }

    // widget_data_t* window_t::create_new_widget(const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept {
    //   auto ptr = widget_stack.create<widget_data_t>(last, bounds, weights, std::move(func));
    //   last = ptr;
    //   return ptr;
    // }

    // widget_data_t* window_t::create_new_widget(const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept {
    //   auto ptr = widget_stack.create<widget_data_t>(last, bounds, inner_bounds, std::move(func));
    //   last = ptr;
    //   return ptr;
    // }

    // widget_data_t* window_t::create_new_widget(const rect &bounds, layout_i* l) noexcept {
    //   last = widget_stack.create<widget_data_t>(last, bounds, content, std::move(func));
    //   return last;
    // }

    // теперь у нас есть юниты, нам бы сначала задать размер в них
    // а потом из них посчитать абсолютные значения

    static vec2 make_extent(const abs_rect &bounds, const abs_rect &next) noexcept {
      auto ext = next.extent;
      if (std::abs(ext.x) < SUI_EPSILON) { ext.x = bounds.extent.x - offset.x; }
      if (std::abs(ext.y) < SUI_EPSILON) { ext.y = bounds.extent.y - offset.y; }
      if (ext.x < 0) { ext.x += bounds.extent.x - offset.x; }
      if (ext.y < 0) { ext.y += bounds.extent.y - offset.y; }
      return ext;
    }

    // нужно еще как нибудь с офсетом поработать, например на крайних значениях очень легко
    // а вот на центральных непонятно что делать, тип делить на половину и менять знак?
    static abs_rect compute_top_left(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(0,0) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_top_center(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(bounds.extent.x / 2 - ext.x / 2, 0) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_top_right(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(bounds.extent.x - ext.x, 0) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_middle_left(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(0, bounds.extent.y / 2 - ext.y / 2) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_middle_center(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = bounds.extent / 2 - ext / 2 + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_middle_right(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(bounds.extent.x - ext.x, bounds.extent.y / 2 - ext.y / 2) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_bottom_left(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(0, bounds.extent.y - ext.y) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_bottom_center(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = vec2(bounds.extent.x / 2 - ext.x / 2, bounds.extent.y - ext.y) + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect compute_bottom_right(const abs_rect &bounds, const abs_rect &next) noexcept {
      const auto ext = make_extent(bounds, next);
      const auto offset = bounds.extent - ext + next.offset;
      return abs_rect(bounds.offset+offset, ext);
    }

    static abs_rect add_scrolls(const abs_rect &bounds, const vec2 &scrolls) noexcept {
      return abs_rect(bounds.offset - scrolls, bounds.extent);
    }

    static abs_rect ns_top_left(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = outer.offset;
      const auto p2 = inner.offset;
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_top_center(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(inner.offset.x, outer.offset.y);
      const auto p2 = vec2(inner.offset.x + inner.extent.x, inner.offset.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_top_right(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(inner.offset.x + inner.extent.x, outer.offset.y);
      const auto p2 = vec2(outer.offset.x + outer.extent.x, inner.offset.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_middle_left(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(outer.offset.x, inner.offset.y);
      const auto p2 = vec2(inner.offset.x, inner.offset.y+inner.extent.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_middle_center(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto pos = inner.offset;
      const auto ext = inner.extent;
      return abs_rect(pos, ext);
    }

    static abs_rect ns_middle_right(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(inner.offset.x + inner.extent.x, inner.offset.y);
      const auto p2 = vec2(outer.offset.x + outer.extent.x, inner.offset.y+inner.extent.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_bottom_left(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(outer.offset.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(inner.offset.x, outer.offset.y + outer.extent.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_bottom_center(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(inner.offset.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(inner.offset.x+inner.extent.x, outer.offset.y+outer.extent.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    static abs_rect ns_bottom_right(const abs_rect &outer, const abs_rect &inner) noexcept {
      const auto p1 = vec2(inner.offset.x+inner.extent.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(outer.offset.x+outer.extent.x, outer.offset.y+outer.extent.y);
      const auto ext = abs(p2 - p1);
      return abs_rect(p1, ext);
    }

    layout_default::layout_default(const abs_rect &bounds, const relative_to content) noexcept :
      layout_i(bounds), content(content)
    {}

    // тут мы можем передать next в формате (1,1,-1,-1), это должен быть отступ в пикселях
    abs_rect layout_default::compute_next(const abs_rect &next) {
      counter += 1;

      rect final_rect = next;
      switch (content) {
        case relative_to::top_left     : final_rect = compute_top_left     (bounds, final_rect); break;
        case relative_to::top_center   : final_rect = compute_top_center   (bounds, final_rect); break;
        case relative_to::top_right    : final_rect = compute_top_right    (bounds, final_rect); break;
        case relative_to::middle_left  : final_rect = compute_middle_left  (bounds, final_rect); break;
        case relative_to::middle_center: final_rect = compute_middle_center(bounds, final_rect); break;
        case relative_to::middle_right : final_rect = compute_middle_right (bounds, final_rect); break;
        case relative_to::bottom_left  : final_rect = compute_bottom_left  (bounds, final_rect); break;
        case relative_to::bottom_center: final_rect = compute_bottom_center(bounds, final_rect); break;
        case relative_to::bottom_right : final_rect = compute_bottom_right (bounds, final_rect); break;
        default: utils::error("Enum is not supported {}", static_cast<size_t>(content));
      }

      // max_size здесь буквально выполняет роль content_bounds, лучше наверное все таки использовать его
      //max_size = max(max_size, final_rect.offset+final_rect.extent);
      content_bounds = content_bounds.extent_to_contain(final_rect);
      last_rect = final_rect; // тут last_rect ни к чему

      return final_rect;
    }

    layout_row::layout_row(const rect &bounds, const std::span<float> &weights) noexcept :
      layout_i(bounds), weights_summ(0.0f), count(weights.size())
    {
      memcpy(this->weights.data(), weights.data(), weights.size()*sizeof(float));
      for (size_t i = 0; i < count; ++i) {
        this->weights_summ += this->weights[i];
      }
    }

    // короче тут есть несколько проблем: нам нужно сохранить изначальные размеры виджета
    // + запомнить его новый размер + скорее всего нужно передавать сюда относительный размер
    // ко всему прочему нужно сделать последовательное размещение виджетов друг за другом
    // новый размер мы используем чтобы разместить после него другой виджет,
    // при этом мы должны обновить данные у виджета выше по иерархии (тип размер контент изменился)
    // это веса, но у нас еще могут быть, просто размеры в разных значениях
    abs_rect layout_row::compute_next(const abs_rect &next) {
      const size_t cur = counter;
      counter += 1;

      const size_t row_num   = cur / count;
      const size_t row_index = cur % count;

      // могут ли веса быть отрицательными?
      // если у нас есть offset то мы можем неиспользовать цикл
      const float width_len_rel = weights_summ / weights[row_index];
      //const float accumulated_width = offset.x;
      //offset.x += width_len_rel;
      //float accumulated_width = 0.0f;
      for (size_t i = 0; i < row_index; ++i) {
        const float width_rel = weights_summ / weights[i];
        accumulated_width += bounds.extent.x * width_rel;
      }

      // как высоту посчитать? чет я еще не придумал, но по идее это максимальная высота детей в строке
      // наверное доступна в лайауте? высота должна браться по размерам виджета
      const float box_offset_y = bounds.extent.y * row_num;
      //offset.y = box_offset_y;

      const auto comp_b = abs_rect(
        vec2(bounds.offset.x+accumulated_width, bounds.offset.y+box_offset_y),
        vec2(bounds.extent.x*width_len_rel,     bounds.extent.y)
      );

      // в next мы можем указать например отступы тип ((1,1),(-1,-1))
      // нужно учесть это дело в функциях
      //bounds = bounds.extent_to_contain(comp_b);
      const auto computed = compute_top_left(comp_b, next);
      content_bounds = content_bounds.extent_to_contain(computed);
      //max_size = max(max_size, computed.offset+computed.extent);
      last_rect = computed;
      return computed;
      //return comp_b;
    }

    layout_column::layout_column(const rect &bounds, const std::span<float> &weights) noexcept :
      layout_i(bounds), weights_summ(0.0f), count(weights.size())
    {
      memcpy(this->weights.data(), weights.data(), weights.size()*sizeof(float));
      for (size_t i = 0; i < count; ++i) {
        this->weights_summ += this->weights[i];
      }
    }

    abs_rect layout_column::compute_next(const abs_rect &next) {
      const size_t cur = counter;
      counter += 1;

      const size_t column_num   = cur / count;
      const size_t column_index = cur % count;

      const float height_len_rel = weights_summ / weights[column_index];
      float accumulated_height = 0.0f;
      for (size_t i = 0; i < column_index; ++i) {
        const float width_rel = weights_summ / weights[i];
        accumulated_height += bounds.extent.y * width_rel;
      }

      const float box_offset_x = bounds.extent.x * row_num;

      const auto comp_b = abs_rect(
        vec2(bounds.offset.x+box_offset_x, bounds.offset.y+accumulated_height),
        vec2(bounds.extent.x,              bounds.extent.y*height_len_rel)
      );

      const auto computed = compute_top_left(comp_b, next);
      content_bounds = content_bounds.extent_to_contain(computed);
      //max_size = max(max_size, computed.offset+computed.extent);
      last_rect = computed;
      return computed;
    }

    // в каком формате будет inner_bounds? сейчас он в абсолютных значениях
    // пусть он здесь так остается, но в функцию мы положим относительные размеры 
    layout_nine_slice::layout_nine_slice(const rect &bounds, const rect &inner_bounds) noexcept :
      layout_i(bounds), inner_bounds(inner_bounds)
    {}
    
    abs_rect layout_nine_slice::compute_next(const abs_rect &next) {
      // тут нужно предусмотреть ТОЛЬКО 9 детей
      // хотя возможно нужно сделать это ограничение легким
      const size_t cur = counter; // % static_cast<size_t>(relative_to::count)
      counter += 1;

      rect final_rect;
      switch(static_cast<relative_to>(cur)) {
        case relative_to::top_left     : final_rect = compute_top_left(ns_top_left     (bounds, inner_bounds), next); break;
        case relative_to::top_center   : final_rect = compute_top_left(ns_top_center   (bounds, inner_bounds), next); break;
        case relative_to::top_right    : final_rect = compute_top_left(ns_top_right    (bounds, inner_bounds), next); break;
        case relative_to::middle_left  : final_rect = compute_top_left(ns_middle_left  (bounds, inner_bounds), next); break;
        case relative_to::middle_center: final_rect = compute_top_left(ns_middle_center(bounds, inner_bounds), next); break;
        case relative_to::middle_right : final_rect = compute_top_left(ns_middle_right (bounds, inner_bounds), next); break;
        case relative_to::bottom_left  : final_rect = compute_top_left(ns_bottom_left  (bounds, inner_bounds), next); break;
        case relative_to::bottom_center: final_rect = compute_top_left(ns_bottom_center(bounds, inner_bounds), next); break;
        case relative_to::bottom_right : final_rect = compute_top_left(ns_bottom_right (bounds, inner_bounds), next); break;
        default: utils::error("Enum is not supported");
      }

      // найнслайс наверное должен быть сделан так чтобы не допускать выходов за пределы внешнего бокса
      // то есть иннер бокс должен быть строго внутри основного бокса
      //max_size = max(max_size, final_rect.offset+final_rect.extent);
      content_bounds = content_bounds.extent_to_contain(final_rect);
      last_rect = final_rect;
      return final_rect;
    }

    window_t* next_window(window_t* prev, window_t* start) const noexcept {
      return utils::ring::list_next<0>(prev, start);
    }

    window_t* context::find_window(const size_t hash) const noexcept {
      auto w = windows;
      for (; w != nullptr && w->hash != hash; w = next_window(w, windows)) {}
      return w;
    }

    window_t* context::create_window(const size_t hash, const std::string_view &str) noexcept {
      auto w = window_pool.create(str);
      w->hash = hash;
      utils::ring::list_radd<0>(windows, w);
      return w;
    }

    void context::remove_window(window_t* w) noexcept {
      window_pool.destroy(w);
    }
  }
}
