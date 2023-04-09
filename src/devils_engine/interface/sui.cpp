#include "sui.h"

namespace devils_engine {
  namespace sui {
    using namespace sui::literals;

    float unit::get(const context* ctx) const noexcept {
      const float mult = ctx->unit.mults[static_cast<size_t>(type)];
      return get(mult);
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

      window_t* w;
      auto [ ptr, data_index ] = w->widget_stack.create<widget_data_t>();
      w->last = ptr;

      // как хранить данные окна? окно не удаляется если мы его не видим по идее
      // значит мы там храним все старые данные
      // хотя наверное все равно надо бы как то понять что окно нужно подчистить
      // по идее нужно каждый раз вызывать эту функцию для того чтобы окно существовало
      ptr->layout.bounds = bounds;

      // окно может быть закрыто, тогда трогать его ни к чему

      // где расчитывается extent? при запуске бокса или при разрушении elem?
      const auto elem = box(ctx, extent(300_dp, 1.0_frac));

      return raii_elem(ctx);
    }

    raii_elem box(context* ctx, const rect &bounds, const relative_to content) {
      // в текущее окно нужно добавить бокс, когда считаем размеры?
      // насколько это вообще необходимо считать размеры бокса по детям? это скорее удобно
      //
    }

    raii_elem box(context* ctx, const extent &bounds, const relative_to content) {
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

      // надо вынести в отдельную функцию где мы вернем посчитанный контент баундс
      float w = bounds.w.get(ctx);
      float h = bounds.h.get(ctx);
      if (bounds.w.type == unit::type::relative) {
        w = w * prev_widget->layout->bounds.extent.x;
      } else if (bounds.w.type == unit::type::not_specified) {
        w = prev_widget->layout->bounds.extent.x;
      }

      if (bounds.h.type == unit::type::relative) {
        h = h * prev_widget->layout->bounds.extent.y;
      } else if (bounds.h.type == unit::type::not_specified) {
        h = prev_widget->layout->bounds.extent.y;
      }

      // ну да еще нужно посчитать контент баундс для предыдущего виджета
      // и как то учесть незаданные заранее размеры бокса (хотя это по идее полный размер + ресайз в энде)
      const auto r = rect(vec2(0, 0), vec2(w, h)); // размеры
      // вызываем compute_next в end'е
      // когда мы хотим чтобы виджет менял свои размеры относительно детей?
      const auto pos = prev_widget->layout->compute_next(r); // абсолютный размер и позиция
      // bounds здесь - это content_bounds виджета, а pos это непосредственно размер и позиция следующего виджета
      // также может пригодится передавать относительные значения в лэйаут, как это правильно сделать
      // вообще наверное удачнее предрассчитать абсолютные значения в лэйауте
      // что делать с отступом, у расположения может быть отступ который например вообще имеет смысл в боксах
      // но в лэйаут ров например уже непонятен, по идее можно оставить, единственное что нам нужно - это
      // посчитать правильно ножницы (внутренее объединение двух прямоугольников)


      // было бы неплохо контроллировать общий аллокатор для всей гуишки
      //auto cur = cur_window->create_new_widget(pos, content, default_layout);
      auto l = ctx->window_data.create<layout_default>(r, content);
      const auto scissor = pos.shrink_to_intersection(r);

      // что тут? вот мы создали бокс и теперь следующие виджеты или контент
      // займут эту область

      return raii_elem(ctx);
    }

    void end(context* ctx) {
      // тут мы по идее у текущего окна должны "закрыть" предыдущий виджет
      // или закрыть само окно и положить команду к отрисовке?
      // в наклире в энде автор дорисовывал окно - не лучшая идея на мой взгляд
      // что такое закрыть с моей точки зрения? в энде мы имеем полное представление о том
      // сколько места у нас занимает контент и соответственно можем нарисовать виджет
      // по размеру контента, поэтому наверное в энде мы должны сформировать
      // команду к отрисовке и вытащить текущий виджет из стека

      // тут мы расчитаем размер контента, запомним его если есть хеш, запомним прокрутку
      // запомним еще какие нибудь вещи

      auto cur_window = ctx->current;
      utils_assertc(cur_window != nullptr, "probably forgot to create a window");
      auto cur_widget = cur_window->last;
      utils_assertc(cur_widget != nullptr, "current widget not found");

      const auto widget_size = cur_widget->layout->bounds;
      // нужно как то посчитать абсолютную позицию, к сожалению нам обязательно нужно запомнить индекс виджета
      // если мы хотим правильно посчитать позицию тут
      const auto absolute_size = cur_widget->prev->layout->compute_next(widget_size);

      // тут мы узнаем что у виджета есть контент
      const bool has_content = true;
      if (has_content) {

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

    layout_t::layout_t(const rect &bounds, const relative_to content, layout_func_t func) noexcept :
      bounds(bounds), counter(0), maximum_child_size(0,0), content(content), weights_summ(0.0f), inc_counter_and_get(std::move(func))
    {}

    layout_t::layout_t(const rect &bounds, const std::span<float> &weights, layout_func_t func) noexcept :
      bounds(bounds), counter(0), maximum_child_size(0,0),
      content(relative_to::top_left), weights_summ(0.0f), weights(weights),
      inc_counter_and_get(std::move(func))
    {}

    layout_t::layout_t(const rect &bounds, const rect &inner_bounds, layout_func_t func) noexcept :
      bounds(bounds), counter(0), maximum_child_size(0,0),
      inner_bounds(inner_bounds), content(relative_to::top_left), weights_summ(0.0f),
      inc_counter_and_get(std::move(func))
    {}

    widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept :
      layout(bounds, content, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
    {
      // вообще нужно тут input_state посчитать
    }

    widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept :
      layout(bounds, weights, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
    {}

    widget_data_t::widget_data_t(widget_data_t* prev, const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept :
      layout(bounds, inner_bounds, std::move(func)), content_pos(0,0), input_state(0), prev(prev)
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

    widget_data_t* window_t::create_new_widget(const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept {
      last = widget_stack.create<widget_data_t>(last, bounds, content, std::move(func));
      return last;
    }

    widget_data_t* window_t::create_new_widget(const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept {
      auto ptr = widget_stack.create<widget_data_t>(last, bounds, weights, std::move(func));
      last = ptr;
      return ptr;
    }

    widget_data_t* window_t::create_new_widget(const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept {
      auto ptr = widget_stack.create<widget_data_t>(last, bounds, inner_bounds, std::move(func));
      last = ptr;
      return ptr;
    }

    // теперь у нас есть юниты, нам бы сначала задать размер в них
    // а потом из них посчитать абсолютные значения

    // нужно еще как нибудь с офсетом поработать, например на крайних значениях очень легко
    // а вот на центральных непонятно что делать, тип делить на половину и менять знак?
    rect compute_top_left(const rect &bounds, const rect &next) {
      const auto offset = next.offset;
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_top_center(const rect &bounds, const rect &next) {
      const auto offset = vec2(bounds.extent.x / 2 - next.extent.x / 2, next.offset.y);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_top_right(const rect &bounds, const rect &next) {
      const auto offset = vec2(bounds.extent.x - next.extent.x - next.offset.x, next.offset.y);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_middle_left(const rect &bounds, const rect &next) {
      const auto offset = vec2(next.offset.x, bounds.extent.y / 2 - next.extent.y / 2);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_middle_center(const rect &bounds, const rect &next) {
      const auto offset = bounds.extent / 2 - next.extent / 2;
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_middle_right(const rect &bounds, const rect &next) {
      const auto offset = vec2(bounds.extent.x - next.extent.x - next.offset.x, bounds.extent.y / 2 - next.extent.y / 2);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_bottom_left(const rect &bounds, const rect &next) {
      const auto offset = vec2(next.offset.x, bounds.extent.y - next.extent.y);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_bottom_center(const rect &bounds, const rect &next) {
      const auto offset = vec2(bounds.extent.x / 2 - next.extent.x / 2, bounds.extent.y - next.extent.y - next.offset.y);
      return rect(bounds.offset+offset, next.extent);
    }

    rect compute_bottom_right(const rect &bounds, const rect &next) {
      const auto offset = bounds.extent - next.extent - next.offset;
      return rect(bounds.offset+offset, next.extent);
    }

    rect add_scrolls(const rect &bounds, const vec2 &scrolls) {
      return rect(bounds.offset - scrolls, bounds.extent);
    }

    // в джетпак композе мы задавали размеры в качесте модификатора к следующему виджету
    // ну хотя тут мы примерно тоже самое делаем
    rect default_layout(layout_t &l, const rect &next) {
      l.counter += 1;

      rect final_rect;
      switch (l.content) {
        case relative_to::top_left     : final_rect = compute_top_left     (l.bounds, next); break;
        case relative_to::top_center   : final_rect = compute_top_center   (l.bounds, next); break;
        case relative_to::top_right    : final_rect = compute_top_right    (l.bounds, next); break;
        case relative_to::middle_left  : final_rect = compute_middle_left  (l.bounds, next); break;
        case relative_to::middle_center: final_rect = compute_middle_center(l.bounds, next); break;
        case relative_to::middle_right : final_rect = compute_middle_right (l.bounds, next); break;
        case relative_to::bottom_left  : final_rect = compute_bottom_left  (l.bounds, next); break;
        case relative_to::bottom_center: final_rect = compute_bottom_center(l.bounds, next); break;
        case relative_to::bottom_right : final_rect = compute_bottom_right (l.bounds, next); break;
        default: utils::error("Enum is not supported");
      }

      l.bounds = l.bounds.extent_to_contain(final_rect); // ???

      return final_rect;
    }

    rect ns_top_left(const rect &outer, const rect &inner) {
      const auto p1 = outer.offset;
      const auto p2 = inner.offset;
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_top_center(const rect &outer, const rect &inner) {
      const auto p1 = vec2(inner.offset.x, outer.offset.y);
      const auto p2 = vec2(inner.offset.x + inner.extent.x, inner.offset.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_top_right(const rect &outer, const rect &inner) {
      const auto p1 = vec2(inner.offset.x + inner.extent.x, outer.offset.y);
      const auto p2 = vec2(outer.offset.x + outer.extent.x, inner.offset.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_middle_left(const rect &outer, const rect &inner) {
      const auto p1 = vec2(outer.offset.x, inner.offset.y);
      const auto p2 = vec2(inner.offset.x, inner.offset.y+inner.extent.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_middle_center(const rect &outer, const rect &inner) {
      const auto pos = inner.offset;
      const auto ext = inner.extent;
      return rect(pos, ext);
    }

    rect ns_middle_right(const rect &outer, const rect &inner) {
      const auto p1 = vec2(inner.offset.x + inner.extent.x, inner.offset.y);
      const auto p2 = vec2(outer.offset.x + outer.extent.x, inner.offset.y+inner.extent.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_bottom_left(const rect &outer, const rect &inner) {
      const auto p1 = vec2(outer.offset.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(inner.offset.x, outer.offset.y + outer.extent.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_bottom_center(const rect &outer, const rect &inner) {
      const auto p1 = vec2(inner.offset.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(inner.offset.x+inner.extent.x, outer.offset.y+outer.extent.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    rect ns_bottom_right(const rect &outer, const rect &inner) {
      const auto p1 = vec2(inner.offset.x+inner.extent.x, inner.offset.y+inner.extent.y);
      const auto p2 = vec2(outer.offset.x+outer.extent.x, outer.offset.y+outer.extent.y);
      const auto ext = abs(p2 - p1);
      return rect(p1, ext);
    }

    // предполагаем что каждый раздел слайса начинается слева вверху? наверное самое удачное что мы можем сделать
    rect nine_slice_layout(layout_t &l, const rect &next) {
      // тут нужно предусмотреть ТОЛЬКО 9 детей
      // хотя возможно нужно сделать это ограничение легким
      const size_t cur = l.counter; // % static_cast<size_t>(relative_to::count)
      l.counter += 1;

      rect final_rect;
      switch(static_cast<relative_to>(cur)) {
        case relative_to::top_left     : final_rect = compute_top_left(ns_top_left     (l.bounds, l.inner_bounds), next); break;
        case relative_to::top_center   : final_rect = compute_top_left(ns_top_center   (l.bounds, l.inner_bounds), next); break;
        case relative_to::top_right    : final_rect = compute_top_left(ns_top_right    (l.bounds, l.inner_bounds), next); break;
        case relative_to::middle_left  : final_rect = compute_top_left(ns_middle_left  (l.bounds, l.inner_bounds), next); break;
        case relative_to::middle_center: final_rect = compute_top_left(ns_middle_center(l.bounds, l.inner_bounds), next); break;
        case relative_to::middle_right : final_rect = compute_top_left(ns_middle_right (l.bounds, l.inner_bounds), next); break;
        case relative_to::bottom_left  : final_rect = compute_top_left(ns_bottom_left  (l.bounds, l.inner_bounds), next); break;
        case relative_to::bottom_center: final_rect = compute_top_left(ns_bottom_center(l.bounds, l.inner_bounds), next); break;
        case relative_to::bottom_right : final_rect = compute_top_left(ns_bottom_right (l.bounds, l.inner_bounds), next); break;
        default: utils::error("Enum is not supported");
      }

      // найнслайс наверное должен быть сделан так чтобы не допускать выходов за пределы внешнего бокса
      // то есть иннер бокс должен быть строго внутри основного бокса
      return final_rect;
    }

    layout_default::layout_default(const rect &bounds, const relative_to content) noexcept :
      layout_i(bounds), content(content)
    {}

    rect layout_default::compute_next(const rect &next) {
      counter += 1;

      rect final_rect;
      switch (content) {
        case relative_to::top_left     : final_rect = compute_top_left     (bounds, next); break;
        case relative_to::top_center   : final_rect = compute_top_center   (bounds, next); break;
        case relative_to::top_right    : final_rect = compute_top_right    (bounds, next); break;
        case relative_to::middle_left  : final_rect = compute_middle_left  (bounds, next); break;
        case relative_to::middle_center: final_rect = compute_middle_center(bounds, next); break;
        case relative_to::middle_right : final_rect = compute_middle_right (bounds, next); break;
        case relative_to::bottom_left  : final_rect = compute_bottom_left  (bounds, next); break;
        case relative_to::bottom_center: final_rect = compute_bottom_center(bounds, next); break;
        case relative_to::bottom_right : final_rect = compute_bottom_right (bounds, next); break;
        default: utils::error("Enum is not supported {}", static_cast<size_t>(content));
      }

      content_bounds = content_bounds.extent_to_contain(final_rect);

      return final_rect;
    }

    layout_row::layout_row(const rect &bounds, const std::span<float> &weights) noexcept :
      layout_i(bounds), weights_summ(0.0f), count(weights.size()), elem_bounds(bounds)
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
    rect layout_row::compute_next(const rect &next) {
      const size_t cur = counter;
      counter += 1;

      const size_t row_num   = cur / count;
      const size_t row_index = cur % count;

      const float width_len_rel = weights_summ / weights[row_index];
      float accumulated_width = 0.0f;
      for (size_t i = 0; i < row_index; ++i) {
        const float width_rel = weights_summ / weights[i];
        accumulated_width += elem_bounds.extent.x * width_rel;
      }

      // как высоту посчитать? чет я еще не придумал, но по идее это максимальная высота детей в строке
      // наверное доступна в лайауте? высота должна браться по размерам виджета
      const float box_offset_y = elem_bounds.extent.y * row_num;
      offset.y = box_offset_y;

      const auto comp_b = rect(
        vec2(elem_bounds.offset.x+accumulated_width, elem_bounds.offset.y+box_offset_y),
        vec2(elem_bounds.extent.x*width_len_rel,     elem_bounds.extent.y) // l.bounds.extent.y
      );

      bounds = bounds.extent_to_contain(comp_b);
      const auto computed = compute_top_left(comp_b, next);
      content_bounds = content_bounds.extent_to_contain(computed);
      return computed;
    }

    layout_column::layout_column(const rect &bounds, const std::span<float> &weights) noexcept :
      layout_i(bounds), weights_summ(0.0f), count(weights.size())
    {
      memcpy(this->weights.data(), weights.data(), weights.size()*sizeof(float));
      for (size_t i = 0; i < count; ++i) {
        this->weights_summ += this->weights[i];
      }
    }

    rect layout_column::compute_next(const rect &next) {
      const size_t cur = counter;
      counter += 1;

      const size_t column_num   = cur / count;
      const size_t column_index = cur % count;

      // как высоту посчитать? чет я еще не придумал, но по идее это максимальная высота детей в строке
      // наверное доступна в лайауте? высота должна браться по размерам виджета
      const float box_offset_x = offset.x;
      offset.x += next.offset.x;

      const float height_len_rel = weights_summ / weights[column_index];
      float accumulated_height = 0.0f;
      for (size_t i = 0; i < column_index; ++i) {
        const float width_rel = weights_summ / weights[i];
        accumulated_height += bounds.extent.y * width_rel;
      }

      const auto comp_b = rect(
        vec2(bounds.offset.x+box_offset_x,   bounds.offset.y+accumulated_height),
        vec2(bounds.extent.x*height_len_rel, bounds.extent.y) // l.bounds.extent.y
      );
      return compute_top_left(comp_b, next);
    }

    layout_nine_slice::layout_nine_slice(const rect &bounds, const rect &inner_bounds) noexcept :
      layout_i(bounds), inner_bounds(inner_bounds)
    {}

    rect layout_nine_slice::compute_next(const rect &next) {
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
      return final_rect;
    }

    // располагаем элементы друг за другом в линию в количестве l.weights.size() штук
    // затем начинаем новую линию которая ниже предыдущей на 1.0 * высоту виджета
    // если размеры предыдущего виджета больше, то линии будут просто наслаиваться друг на друга вниз,
    // если же нет, то линия обрежется по границе предыдущего виджета и у предыдущего виджета может появиться полоса прокрутки
    // конкретно этот вариант позволяет только расположить виджеты друг за другом посчитав их размер заранее
    // мне нужен еще способ расположить друг за другом виджеты произвольно размера, как это сделать?
    // по идее вместе с счетчиком мне нужно увеличивать отступ
    rect row_layout(layout_t &l, const rect &next) {
      const size_t cur = l.counter;
      l.counter += 1;

      const size_t row_num   = cur / l.weights.size();
      const size_t row_index = cur % l.weights.size();

      // как высоту посчитать? чет я еще не придумал, но по идее это максимальная высота детей в строке
      // наверное доступна в лайауте? высота должна браться по размерам виджета
      const float box_height = row_num * l.maximum_child_size.y;

      const float width_len_rel = l.weights_summ / l.weights[row_index];
      float accumulated_width = 0.0f;
      for (size_t i = 0; i < row_index; ++i) {
        const float width_rel = l.weights_summ / l.weights[i];
        accumulated_width += l.bounds.extent.x * width_rel;
      }

      const auto comp_b = rect(
        vec2(l.bounds.offset.x+accumulated_width, l.bounds.offset.y),
        vec2(l.bounds.extent.x*width_len_rel,     box_height) // l.bounds.extent.y
      );
      return compute_top_left(comp_b, next);
    }

    rect column_layout(layout_t &l, const rect &next) {
      const size_t cur = l.counter;
      l.counter += 1;

      const size_t column_num   = cur / l.weights.size();
      const size_t column_index = cur % l.weights.size();

      // как посчитать ширину?
      const float box_width = column_num * l.maximum_child_size.x;

      const float height_len_rel = l.weights_summ / l.weights[column_index];
      float accumulated_height = 0.0f;
      for (size_t i = 0; i < column_index; ++i) {
        const float height_rel = l.weights_summ / l.weights[i];
        accumulated_height += l.bounds.extent.y * height_rel;
      }

      const auto comp_b = rect(
        vec2(l.bounds.offset.x, l.bounds.offset.y+accumulated_height),
        vec2(box_width,         l.bounds.extent.y*height_len_rel) //l.bounds.extent.x
      );
      return compute_top_left(comp_b, next);
    }
  }
}
