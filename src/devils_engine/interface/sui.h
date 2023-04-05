#ifndef DEVILS_ENGINE_INTERFACE_DGUI_H
#define DEVILS_ENGINE_INTERFACE_DGUI_H

#include <string_view>
#include <span>
#include <functional>
// хотя стак в конечном итоге лучше сделать иначе
#include <stack>
#include <vector>

#include "utils/core.h"
#include "utils/type_traits.h"
#include "utils/list.h"
#include "utils/dynamic_allocator.h"
#include "utils/block_allocator.h"
#include "utils/arena_allocator.h"

namespace devils_engine {
  // нужно сменить название, чет dgui в куче мест используется
  // чат джпт посоветовал использовать CarbonUI, что неплохо
  // можно потроллить язык карбон, но на самом деле надо бы придумать что то еще
  // было бы неплохо использовать слово на S и добавить UI чтобы сокращение получилось sui
  // например SereneUI, StarUI, SuppleUI
  // может быть реально использовать SuppleUI
  namespace sui {
    struct context;

    void end(context* ctx);

    struct raii_elem {
      context* ctx;
      bool value;

      constexpr raii_elem(context* ctx, const bool value = true) noexcept : ctx(ctx), value(value) {}
      inline ~raii_elem() noexcept { if (ctx != nullptr) end(ctx); }

      constexpr operator bool () const noexcept { return value; }

      constexpr raii_elem(raii_elem &&move) noexcept : ctx(move.ctx), value(move.value) { move.ctx = nullptr; }
      constexpr raii_elem & operator=(raii_elem &&move) noexcept {
        if (ctx != nullptr) end(ctx);
        ctx = move.ctx;
        value = move.value;
        move.ctx = nullptr;
        return *this;
      }

      constexpr raii_elem(const raii_elem &copy) noexcept = delete;
      constexpr raii_elem & operator=(const raii_elem &copy) noexcept = delete;
    };

    enum class relative_to {
      top_left,   // 0,0 выставляется в левом верхнем углу
      top_center, // по центру сверху и так далее
      top_right,
      middle_left,
      middle_center,
      middle_right,
      bottom_left,
      bottom_center,
      bottom_right,
      count
    };

    struct handle {
      constexpr static const size_t underlying_size = 16;

      // тип?
      size_t type;
      uint8_t mem[underlying_size];

      constexpr handle() noexcept : type(utils::type_id<void>()) {}
      constexpr handle(const handle &copy) noexcept = default;
      constexpr handle(handle &&move) noexcept = default;
      constexpr handle & operator=(const handle &copy) noexcept = default;
      constexpr handle & operator=(handle &&move) noexcept = default;

      template <typename T>
      handle(T&& obj) noexcept : type(utils::type_id<T>()) { place_object(std::forward<T>(obj)); }
      template <typename T>
      handle & operator=(T&& obj) noexcept { type = utils::type_id<T>(); place_object(std::forward<T>(obj)); }

      template <typename T>
      T get() const {
        utils_assertf(is<T>(), "Expected {} ({}), hold {}", utils::type_id<T>(), utils::type_name<T>(), type);
        return *reinterpret_cast<T*>(mem);
      }

      constexpr bool is_valid() const noexcept { return type != utils::type_id<void>(); }
      template <typename T>
      constexpr bool is() const noexcept { return type == utils::type_id<T>(); }

      template <typename T>
      void place_object(T&& obj) {
        static_assert(!std::is_void_v<T>);
        static_assert(std::is_trivially_destructible_v<T> && std::is_trivially_copyable_v<T>,
                      "Object must be trivially destructible and trivially copyable");
        static_assert(sizeof(T) <= underlying_size, "Type is too big for this handle");

        new (mem) T(obj);
      }
    };

    struct vec2 {
      float x, y;

      constexpr vec2() noexcept : x(0.0f), y(0.0f) {}
      constexpr vec2(const float x, const float y) noexcept : x(x), y(y) {}
      constexpr ~vec2() noexcept = default;

      constexpr vec2(const vec2 &v) noexcept = default;
      constexpr vec2(vec2 &&v) noexcept = default;
      constexpr vec2 & operator=(const vec2 &v) noexcept = default;
      constexpr vec2 & operator=(vec2 &&v) noexcept = default;

      const float & operator[] (const size_t i) const noexcept { return i == 0 ? x : y; }
      float & operator[] (const size_t i) noexcept { return i == 0 ? x : y; }

      constexpr size_t length() const noexcept { return 2; }

      constexpr void operator+=(const vec2 &v) noexcept { x += v.x; y += v.y; }
      constexpr void operator-=(const vec2 &v) noexcept { x -= v.x; y -= v.y; }
      constexpr void operator*=(const vec2 &v) noexcept { x *= v.x; y *= v.y; }
      constexpr void operator/=(const vec2 &v) noexcept { x /= v.x; y /= v.y; }

      constexpr void operator+=(const float v) noexcept { x += v; y += v; }
      constexpr void operator-=(const float v) noexcept { x -= v; y -= v; }
      constexpr void operator*=(const float v) noexcept { x *= v; y *= v; }
      constexpr void operator/=(const float v) noexcept { x /= v; y /= v; }
    };

    constexpr vec2 operator+(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x + b.x, a.y + b.y); }
    constexpr vec2 operator-(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x - b.x, a.y - b.y); }
    constexpr vec2 operator*(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x * b.x, a.y * b.y); }
    constexpr vec2 operator/(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x / b.x, a.y / b.y); }

    constexpr vec2 operator+(const vec2 &a, const float b) noexcept { return vec2(a.x + b, a.y + b); }
    constexpr vec2 operator-(const vec2 &a, const float b) noexcept { return vec2(a.x - b, a.y - b); }
    constexpr vec2 operator*(const vec2 &a, const float b) noexcept { return vec2(a.x * b, a.y * b); }
    constexpr vec2 operator/(const vec2 &a, const float b) noexcept { return vec2(a.x / b, a.y / b); }

    constexpr vec2 operator+(const float a, const vec2 &b) noexcept { return vec2(a + b.x, a + b.y); }
    constexpr vec2 operator-(const float a, const vec2 &b) noexcept { return vec2(a - b.x, a - b.y); }
    constexpr vec2 operator*(const float a, const vec2 &b) noexcept { return vec2(a * b.x, a * b.y); }
    constexpr vec2 operator/(const float a, const vec2 &b) noexcept { return vec2(a / b.x, a / b.y); }

    constexpr vec2 abs(const vec2 &a) noexcept { return vec2(std::abs(a.x), std::abs(a.y)); }
    constexpr vec2 max(const vec2 &a, const vec2 &b) noexcept { return vec2(std::max(a.x, b.x), std::max(a.y, b.y)); }
    constexpr vec2 min(const vec2 &a, const vec2 &b) noexcept { return vec2(std::min(a.x, b.x), std::min(a.y, b.y)); }

    struct unit {
      enum class type {
        not_specified,
        absolute,
        density_independent,
        scaled_pixels,
        relative,
        //relative_w, // относительные координаты нужно применить к соответствующему числу дополнительно
        //relative_h, // тогда как остальные ни к чему
        count
      };

      float value;
      enum type type;

      constexpr unit() noexcept : value(0.0f), type(type::not_specified) {}
      constexpr unit(const float value) noexcept : value(value), type(type::absolute) {}
      constexpr unit(const float value, const enum type type) noexcept : value(value), type(type) {}
      // наверное нужно передать контекст, в нем должны храниться рассчитанные константы размеров экрана,
      // то есть необходимый мультипликатор для типа
      constexpr float get(const float mult = 1.0f) const noexcept { return value*mult; }
      float get(const context* ctx) const noexcept;

      //unit() noexcept = default;
      constexpr unit(const unit &copy) noexcept = default;
      constexpr unit(unit &&move) noexcept = default;
      constexpr unit & operator=(const unit &copy) noexcept = default;
      constexpr unit & operator=(unit &&move) noexcept = default;
    };

    namespace literals {
      using float_t = long double;
      using uint_t = unsigned long long;
      constexpr unit operator ""_pct(const float_t num) noexcept { return unit(num / 100.0, unit::type::relative); }
      constexpr unit operator ""_pct(const uint_t num) noexcept { return unit(float_t(num) / 100.0, unit::type::relative); }
      constexpr unit operator ""_frac(const float_t num) noexcept { return unit(num, unit::type::relative); }
      constexpr unit operator ""_dp(const float_t num) noexcept { return unit(num, unit::type::density_independent); }
      constexpr unit operator ""_dp(const uint_t num) noexcept { return unit(num, unit::type::density_independent); }
      constexpr unit operator ""_px(const float_t num) noexcept { return unit(num, unit::type::absolute); }
      constexpr unit operator ""_px(const uint_t num) noexcept { return unit(num, unit::type::absolute); }
      constexpr unit operator ""_sp(const float_t num) noexcept { return unit(num, unit::type::scaled_pixels); }
      constexpr unit operator ""_sp(const uint_t num) noexcept { return unit(num, unit::type::scaled_pixels); }
    }

    struct extent {
      unit w, h;

      constexpr extent() noexcept = default;
      constexpr extent(const float w, const float h) noexcept : w(w), h(h) {}
      constexpr extent(const unit &w, const unit &h) noexcept : w(w), h(h) {}
    };

    // нужно учесть что нам требуется подстроить позицию по размеру бокса
    // поизиция скорее всего нуждается еще в дополнительных данных:
    // относительная позиция - относительно чего? относительно какого угла бокса?
    // наверное для любой позиции это справедливо
    // либо расчет размеров все таки переложить на плечи пользователя?
    // еще большой вопрос: может быть вообще в принципе все размеры относительные по умолчанию?
    // чем это плохо? относительно чего размер и позиция? можно использовать специальные единицы измерения
    // как в джетпак, в общем то скорее всего требуется какие то единицы измерения относительно физического размера экрана
    // а может и нет... ну то есть физический размер окна действительно нужно учитывать чтобы
    // сделать удачное отображение контента на экране
    // оставляем чисто пользователю ебаторию с конкретными единицами измерения
    struct offset {
      unit x, y;

      constexpr offset() noexcept = default;
      constexpr offset(const float x, const float y) noexcept : x(x), y(y) {}
      constexpr offset(const unit &x, const unit &y) noexcept : x(x), y(y) {}
    };

    struct rel_rect {
      struct offset offset;
      struct extent extent;

      constexpr rel_rect() noexcept = default;
      constexpr rel_rect(const struct offset &of, const struct extent &ex) noexcept : offset(of), extent(ex) {}
    };

    struct rect {
      vec2 offset;
      vec2 extent;
      // не уверен где это будет лучше всего раположить
      // + вот это конкретный рект - это уже расчитанные значения
      // или еще нет, по идее расчет конкретного размера виджета - это тоже не наша забота
      // а значит тут алигмент то даже не нужен
      // а как понять якорь для конкретного элемента, элементы то у нас относительно ПРЕДЫДУЩЕГО
      // бокса, а не иначе
      //relative_to alignment;

      // тут надо бы задать разные типы размеров
      // должны быть относительные и абсолютные значения
      // причем и для позиции и для размеров
      // хранить нам наверное это не придется

      // по умолчанию все доступное пространство?
      // нет наверное хотелось бы вычислять размеры некоторых окон по размеру контента
      // что такое контент? это боксы заданных размеров: например текст определенного размера шрифта
      // имеет такой то размер по x,y, то есть зная размер шрифта легко вычислять размер бокса вокруг текста
      constexpr rect() noexcept = default;
      //rect(const vec2 &of);
      //rect(const vec2 &ex);
      constexpr rect(const vec2 &of, const vec2 &ex) noexcept : offset(of), extent(ex) {}
      // const relative_to alignment = relative_to::top_left

      constexpr bool intersect(const vec2 &v) const noexcept {
        return contain(v);
      }

      constexpr bool contain(const vec2 &v) const noexcept {
        const auto to_space = v - offset;
        return to_space.x >= 0 && to_space.x <= extent.x &&
               to_space.y >= 0 && to_space.y <= extent.y;
      }

      constexpr bool intersect(const rect &r) const noexcept {
        const auto center_point_to_space = abs((offset + extent / 2) - (r.offset + r.extent / 2)) * 2;
        const auto ext = extent + r.extent;
        return center_point_to_space.x <= ext.x &&
               center_point_to_space.y <= ext.y;
      }

      constexpr bool contain(const rect &r) const noexcept {
        return contain(r.offset) && contain(r.offset+r.extent);
      }

      constexpr rect extent_to_contain(const rect &r) const noexcept {
        const auto min_p = min(offset, r.offset);
        return rect(min_p, max(offset+extent, r.offset+r.extent) - min_p);
      }
    };

    static_assert(rect(vec2(-1,-1), vec2(6,6)).intersect(rect(vec2(5,5), vec2(2,2))));
    static_assert(rect(vec2(-1,-1), vec2(6,6)).contain(rect(vec2(-1,-1), vec2(6,6))));
    static_assert(rect(vec2(-1,-1), vec2(6,6)).extent_to_contain(rect(vec2(-2,-2), vec2(6,6))).contain(rect(vec2(-2,-2), vec2(7,7))));

    // нам 250% потребуется взять часть изображения, в наклире использовались 2байта переменные
    // в микроюи вообще не использовались картинки, структура уже занимает 48 байт
    struct image {
      struct handle handle;
      vec2 size;
      rect region;

      constexpr image() noexcept : size(0,0) {}
      constexpr image(const struct handle &handle) noexcept : handle(handle), size(0,0) {}
      constexpr image(const struct handle &handle, const vec2 &size, const rect &region) noexcept :
        handle(handle), size(size), region(region)
      {}
    };

    //void init_context(context* ctx);

    // точка входа для гуи - просто бокс с дополнительными настройками
    //
    raii_elem window(context* ctx, const std::string_view &name, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    // надо бы как нибудь уметь изменять размеры окна, и передавать в окно хендл

    // размер может определяться относительно родительского элемента, но и может быть абсолютным
    // у родительского бокса должны быть настройки относительно чего в нем располагается контент
    // у бокса есть размер, размер окна отсечения, размер контента
    // например просто бокс без дополнительной мишуры: размер совпадает с размером окна отсечения и размерами контента
    // бокс с границей - окно отсечения по размеру бокса минус размер границы и кнтент может раполагаться чуть в меньшем окне
    // + в боксе размер контента может быть сильно больше размера бокса - для этого нужно
    // создать слайдеры по бокам в боксе и запомнить их состояние

    // ивенты с инпутом нам доступны по иерархии для всех боксов, но не из всех боксов их вообще имеет смысл брать

    // короче я вот что еще подумал: при вызове функции бокс нужно передавать ей специальную структуру
    // в которой будет содержаться размеры бокса относительно родительского элемента
    // итоговый размер родителя будем расчитывать в функции энд (точнее итоговый размер контента родителя)
    raii_elem box(context* ctx, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    // позволит сохранить текущее положение бокса контента, bounds - это стартовый размер, потом он перезапишется
    raii_elem box(context* ctx, const std::string_view &name, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    raii_elem box(context* ctx, const size_t hash, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    // + боксы должны уметь быстро переопределять стиль отображения виджетов
    // как быть со стилями? стили по идее представляют из себя набор данных
    // которые мы используем чтобы не делать какие то часто повторяющиеся действия
    // например: есть 3 состояния бокса - активный, наведенный и нажатый (несколько нажатых?)
    // мы можем получить конкретное состояние бокса и нарисовать определенный цвет заливки
    // а можем сказать что у нас есть класс боксов у которых эти свойства похожи
    // просто возьми цвета и картинки вот отсюда

    raii_elem box(context* ctx, const extent &bounds, const relative_to content = relative_to::top_left);
    raii_elem box(context* ctx, const std::string_view &name, const extent &bounds, const relative_to content = relative_to::top_left);
    raii_elem box(context* ctx, const size_t hash, const extent &bounds, const relative_to content = relative_to::top_left);

    // тот же бокс но который не сохраняет стейт и не реагирует на инпут
    // наверное по итогу не нужно, инпут будет иерархическим - единственная беда: кнопка в кнопке
    // я бы сказал что кнопка в кнопке - скорее неудачная идея
    raii_elem group(context* ctx, const rect &bounds, const relative_to content = relative_to::top_left);

    // ожидаем последним аргументом веса будущих виджетов
    // эти функции - это не сами боксы это только бокс лайауты, тогда тут размер не нужен
    // эти функции должны последовательно ставить следующие виджеты друг за другом
    // увеличивая content_bounds (скорее всего content_bounds должен увеличиваться автоматически при расчете размеров)
    raii_elem row(context* ctx, const rect &bounds, const std::span<float> &templ);
    raii_elem column(context* ctx, const rect &bounds, const std::span<float> &templ);

    // какой инпут? внутренний бокс + 9 изображений? или ожидаем бокс + 9 виджетов после
    // как мы можем убрать часть границ? просто не рисовать их вообще
    // рендеринг некоторых боксов нужно переделать чтобы нарисовать скругленные края
    // это по идее тоже частично контент, в чем прикол? нужно нарисовать вместо бокса
    // часть окружности + к этому возможно добавится граница, у этой окружности есть
    // важное свойство - она всегда 90 градусов (пол ПИ) это значит что мы можем заранее
    // подготовить точки для этой арки и ничего дополнительно не перевычислять
    // нам нужно решить только две проблемы такого подхода: край полностью залитый
    // и край с границей, полностью залитый край - это по идее точки края + одна точка в углу
    // край с границей - это по идее "путь" но нужно уметь "отодвигать" одну сторону
    // от другой, ну и еще один вариант - генерить точки для всего рендера, что нужно делать и так и сяк
    raii_elem nine_slice(context* ctx, const rect &bounds, const rect &inner_bounds);

    // когда мы делаем функции row, column, nineslice ожидаем ли мы что функции
    // text, image, color будут занимать один слот? или все таки нужно оборачивать в бокс?
    // я бы предположил что второе, то есть чтобы продвинуть счетчик виджета нужно вызвать обязательно функцию бокс
    // а контент (text, image, color) - это чисто контент и он будет использовать предыдущие значения для отрисовки себя

    // размер определяется длиной строки и размером шрифта + нужно ли переводить на другую строку
    // определенно должен быть текст который займет ровно столько места сколько отведено
    // короче говоря чтобы враппинг сделать нормальный в любом случае нужна помощь со стороны боксов
    // я должен создать дополнительный бокс по высоте и ширине текста (или слова в тексте)
    // + алигнмент этих боксов
    template <typename... Args>
    void textf(context* ctx, const std::string_view &format, Args&&... args) {
      // нужно fmt пока что подключить
    }

    void text(context* ctx, const std::string_view &text);

    // наверное нужно добавить еще вариант который бы определялся внешним боксом
    // как сделать цвет для текста? какие то настроечки для виджетов можно добавить дополнительно после
    // в наклире цвет мы просто указывали перед строкой формата, а как быть тут?
    // я могу указать цвет функцией fill после текста, тип текст и корнер - это контент,
    // а изображение и цвет - это контент модификаторы

    // для большинства целей подойдет сделать матрицу и умножить на нее
    // но с другой стороны ее нужно где то хранить и пользователю передать потом
    enum class corner_type {
      top_right,
      top_left,
      bottom_left,
      bottom_right,
      count
    };
    void circle_corner(context* ctx, const corner_type type); // предыдущий виджет - это размеры

    // размер определяется внешним боксом + опять же настроки отображения (кроп, растяжение и проч)
    // также в изображении иногда требуется взять определенный регион
    void decorate(context* ctx, const image &img);

    struct color {
      uint32_t container;

      color(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 0.0f);
    };
    void fill(context* ctx, const color &c); // заливка бокса цветом

    // ничего, пропускаем это место, эквивалентно вызову пустой функции бокс
    void space(context* ctx);

    void end(context* ctx);

    // const float font_size, // тут наверное лучше использовать std span (хотя может быть и нет)
    // для инпута нужно еще понять где у нас стоит курсор и вообще всю математику курсора
    // тут еще нужно рендерить только видимый текст хотя вообще может быть инпут вообще ничего не рисует
    // а рисуем мы сами все нужные вещи для нас... с другой стороны нам надо както кликнуть на инпут
    // как то обозначить что именно в этот инпут мы складываем данные... над короч посмотреть как сделано в наклире
    // было бы неплохо чтобы инпут был максимально обособлен от способа его отрисовки, но при этом
    // мне нужно рисовать курсор и выделение
    // как сделать выделение? для работы с текстом в целом нужно сделать 6 (?) функций
    // клик (ставим курсор), драг (перемещаем выделенный фрагмент), копи (выделенный текст в буфер), кат (удаляем и в буфер),
    // паст (кладем в текущую строку) + добавление символов к строке
    // как курсор нарисовать? это по идее бокс в заданной области
    // курсор по идее существует только для одного места, нам нужно понять что это именно этот инпут
    // по идее мы можем взять указатель на передаваемые структуры - он будет уникальным
    // как понять что это за инпут? то есть нам во первых нужно настроить фильтр
    // а во вторых в некоторых инпутах мы не принимаем символы перевода строки
    // а перевод строки - это скорее дефолтное внешнее действие в данном окне
    // нам этот перевод строки нужно перехватить (хотя нужно ли именно перехватывать?)
    // скорее нам пригодится фильтр
    size_t input(context* ctx, std::string &str, const bool capacity_is_max = true);
    size_t input(context* ctx, std::span<char> &str); // size_t &count

    size_t utf_decode(const std::string_view &str, uint32_t &rune);
    size_t utf_encode(const uint32_t rune,  std::span<char> &sp);

    // еще важно это сохранять стейт некоторых боксов, как это лучше всего сделать?
    // прежде всего меня интересует смена стейта после нажатию кнопки
    // или смена стейта во время нажатия как это сделать?
    // для гуи по большому счету неважно что такое стейт и как именно он меняется
    // в гуи в самом можно просто собирать разного рода информацию, например
    // количество кликов по боксу, время текущего состояния
    // где состояние это активный, наведенный, нажатый, неактивный (надо посмотреть как в наклире это сделано)
    // состояние неактивный нужно ли? для каждого состояния можно запоминать время, точнее для текущего
    // еще к вышесказанному у нас есть понятие активного элемента, который активируется по умолчанию
    // при нажатии энтер
    // что по прогресс барам? прогресс бар - бокс в боксе, размером часть внешнего бокса
    // что по слайдерам? слайдер уже сложнее, нажатие и движение мышки меняют значение переменной
    // в остальном это бокс + текст по центру + несколько переменных
    // что по 9slice? по идее нужно задать размеры внутреннего бокса и расставить 9 картинок
    // как кстати рисовать рамку? я вообще думал на счет рисовать просто два бокса, один поменьше поверх другого
    // но как например внутреннюю часть сделать прозрачной? неужели придется рисовать 4 бокса? или даже 9слайс?
    // вообще наверное лучше найнслайс распространить на это дело, найнслайс нужно научить не рисовать некоторые
    // объекты, например нижнюю часть бордера + на него можно как раз повесить скругление
    // но тем не менее мне нужна прозрачность для слоев внутри окна + общая прозрачность окна

    // код должен выглядеть примерно так:
    // if (const auto w = dgui::window(ctx, ...); w) {
    //   {
    //     const auto elem = dgui::box(ctx, rect(offset(relative_to::center), extent(0.5, 0.5)));
    //     dgiu::image(ctx, handle(img_ptr)); // изображение заднего фона
    //     // желательно конечно понимать сразу сколько "слоев" тут будет,
    //     // с другой строны мы основные вычисления производим в функции end()
    //     // а до этого нам нужно сохранить настройки
    //     // все таки удобно сделать массив с весами заранее
    //     const auto _ = dgui::row(ctx, rect(), { 1.0f, 2.0f, 1.0f });
    //     {
    //       const auto _ = dgui::row_element(ctx, 1);
    //       const auto bounds = dgui::bounds(ctx);
    //     }
    //   }
    // }

    // детали реализации: мы всегда единомоментно можем взаимодействовать только с одним окном
    // и только с одним виджетом (боксом?) этого окна, но при этом окна у нас располагаются по "слоям"
    // то есть у нас есть окна которые находятся впереди, есть окна которые находятся позади
    // + есть текущее активное окно для которого считывается инпут (ну или на которое мы навелись мышкой)
    // окна представляют собой иерархию боксов, в котором у окна заданы боксы дети, у них в свою очередь тоже дети
    // и так далее рекурсивно, в какой то определенный момент может быть фокус только на одном виджете
    // но при этом стейт виджетов я должен хранить между кадрами, что такое стейт?
    // в боксе меня интересует: позиция+размер, предыдущие стейты активности, время активности (какой?), стейт кликов
    // некоторый счетчик (например для row layout)
    // в наклире просто хранится несколько чисел
    // осталось понять как составить команды к отрисовке

    // эта штука появляется уже после того как мы превратили всю отрисовку в вершины и индексы
    struct draw_command_t { // должно быть также просто как и в наклире
      uint32_t elem_count;
      rect clip;
      handle texture;
      handle userdata;
    };

    // имеет смысл сделать виртуальный класс с основными компонентами
    // bounds, counter, offset, content_bounds, а остальные компоненты убрать по
    // наследникам + написать свою реализацию Арены памяти (memory arena)
    // откуда собственно получать память для всех структур ГУИ
    struct layout_t {
      using layout_func_t = std::function<rect(layout_t &, const rect &)>;

      rect bounds; // общий размер
      size_t counter;
      vec2 maximum_child_size;

      // частично можно положить в юнион
      rect inner_bounds; // размер внутреннего поля для nine_slice
      relative_to content;
      // здесь особо не получится расширить на все будущие выдумки, придется так
      float weights_summ;
      std::span<float> weights;
      // как отличить row, column, nine_slice и другие?
      // вообще это все дело отличается по поведению

      layout_func_t inc_counter_and_get;

      layout_t(const rect &bounds, const relative_to content, layout_func_t func) noexcept;
      layout_t(const rect &bounds, const std::span<float> &weights, layout_func_t func) noexcept;
      layout_t(const rect &bounds, const rect &inner_bounds, layout_func_t func) noexcept;
    };

    rect default_layout(layout_t &l, const rect &next);
    rect row_layout(layout_t &l, const rect &next);
    rect column_layout(layout_t &l, const rect &next);
    rect nine_slice_layout(layout_t &l, const rect &next);

    class layout_i {
    public:
      rect bounds;
      rect content_bounds;
      vec2 offset;
      size_t counter;

      inline layout_i(const rect &bounds) noexcept : bounds(bounds), content_bounds(bounds), offset(0,0), counter(0) {}
      virtual ~layout_i() noexcept = default;
      virtual rect compute_next(const rect &next) = 0;
    };

    class layout_default : public layout_i {
    public:
      relative_to content;

      layout_default(const rect &bounds, const relative_to content) noexcept;
      rect compute_next(const rect &next) override;
    };

    // как удалить память? наверное нужно сделать деинит функцию
    // какой еще вариант? предугадать максимум?
    // вообще можно сделать 16 или 32 и пусть по иерархии вызывают функции то есть
    // { const auto a = row_layout(ctx, b, { 1, 2, ...}); { const auto b = row_layout(ctx, b, { 1, 2, ...}); } }
    class layout_row : public layout_i {
    public:
      constexpr static const size_t maximum_weights = 32;

      float weights_summ;
      size_t count;
      std::array<float, maximum_weights> weights; // спан не сработает, нужна отдельная память 

      layout_row(const rect &bounds, const std::span<float> &weights) noexcept;
      rect compute_next(const rect &next) override;
    };

    class layout_column : public layout_i {
    public:
      constexpr static const size_t maximum_weights = 32;

      float weights_summ;
      size_t count;
      std::array<float, maximum_weights> weights; // спан не сработает, нужна отдельная память 

      layout_column(const rect &bounds, const std::span<float> &weights) noexcept;
      rect compute_next(const rect &next) override;
    };

    class layout_row_dyn : public layout_i {
    public:
      float weights_summ;
      std::vector<float> weights;

      layout_row(const rect &bounds, const std::span<float> &weights) noexcept;
      rect compute_next(const rect &next) override;
    };

    class layout_column_dyn : public layout_i {
    public:
      float weights_summ;
      std::vector<float> weights;

      layout_column(const rect &bounds, const std::span<float> &weights) noexcept;
      rect compute_next(const rect &next) override;
    };

    class layout_nine_slice : public layout_i {
    public:
      rect inner_bounds;

      layout_column(const rect &bounds, const rect &inner_bounds) noexcept;
      rect compute_next(const rect &next) override;
    };

    // тут как минимум нужно выделять память для текста + картинка занимает довольно много места
    // но в остальном нас интересует скорее способ отображения этого контента
    // мы должны получить из контента команду к отрисовке
    // скорее всего нужно сделать так же как и в случае с layout то есть интерфес
    // и фабрики-наследники от этого интерфейса
    // либо это уже сразу команда? скорее нет чем да, с другой стороны почему нет?
    class content_i {
    public:
      //???
      struct color color;

      virtual ~content_i() noexcept = default;
      // конфиг? да наверное лучше сделать так чтобы контент выдавал команду
      // а хотя зачем тут вообще хранить контент? у нас функция например fill должна сразу задавать команду
      // сразу команды? наверное
      virtual std::tuple<size_t, size_t> convert(std::span<float> &vertex_buffer, std::span<uint16_t> &index_buffer) noexcept = 0;
    };

    struct content_data_t {
      color c;
      struct image image;
      std::string_view text;
    };

    struct widget_data_t {
      //layout_t layout;
      layout_i* layout;
      //rect content_bounds; // надо в лайаут отправить
      vec2 content_pos;

      // наведение? по идее посчитаем по content_bounds
      uint32_t input_state; // наведение - просто флаги

      // контент? тут нужно выделить память, цвет в принципе можно положить в uint32_t
      // а вот для картинки желательно указать размер + регион
      // если контент есть, то это автоматически означает что мы рисуем текущий виджет
      // значит наверное надо сразу сделать создание команды отрисовки

      // активный/неактивный, стейт нажатия
      // лайаут, контент?
      // мы должны понять какой именно виджет вообще имеет смысл рисовать
      // виджет который имеет смысл рисовать должен содержать в себе контент
      // либо цвет, либо картинку, либо текст
      // что такое текст? это несколько картинок в ряд по определенному правилу
      // ряд и картинка задается шрифтом

      widget_data_t* prev;

      widget_data_t(widget_data_t* prev, const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept;
      widget_data_t(widget_data_t* prev, const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept;
      widget_data_t(widget_data_t* prev, const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept;
    };

    struct window_t : public utils::ring::list<window_t, 0> {
      constexpr static const size_t maximum_window_name_size = 256;

      size_t hash;
      size_t name_size;
      char name_str[maximum_window_name_size];
      // комманд буффер - он разве не общий? видимо нет, для каждого окна отдельно

      // проперти стейт, попап, едит
      // попап явно должен быть, чтобы старый закрыть а новый открыть
      // эдит тоже нужен чтобы понять текущее состояние курсора едитора
      // проперти - не понимаю зачем нужен

      // в стеке должно быть первым само окно, тогда можно неуказывать тут content_bounds
      // может быть стек сделать иначе? хотя можно и вектором ограничить
      // лучше чтонибудь с динамическим выделением памяти
      //std::vector<widget_data_t> stack;
      // мне бегать по этой структуре не придется?
      //utils::block_allocator widget_stack; // тоже блок аллокатор нужен, заменим на арену в контексте
      widget_data_t* last;

      // таблица (?) - по идее это хештаблица, просто использовать flat_hash_map?

      // навигация по окнам (лист) + родительское окно
      window_t* parent;

      window_t(const std::string_view &str) noexcept;
      widget_data_t* create_new_widget(const rect &bounds, const relative_to content, layout_t::layout_func_t func) noexcept;
      widget_data_t* create_new_widget(const rect &bounds, const std::span<float> &weights, layout_t::layout_func_t func) noexcept;
      widget_data_t* create_new_widget(const rect &bounds, const rect &inner_bounds, layout_t::layout_func_t func) noexcept;
    };

    // наклир выделяет аж 19 команд, но мне наверное будет достаточно лишь:
    // ножницы, прямоугольник, текст, картинка, полигон, кривая (?) (насчет последних двух я не уверен)
    // не помню как именно это все дело превращалось в вершины и индесы,
    // но кажется можно сделать просто с помощью виртуальных функций

    struct command_buffer {
      // в каком буфере хранить? может быть в стеке? как предугадать размер памяти? хороший вопрос
      // можно динамический буфер забацать, динамический стек
      struct rect clip;
      int use_clipping;
      handle userdata;
      size_t begin, end, last;
    };

    struct unit_container {
      constexpr static const size_t number_count = static_cast<size_t>(unit::type::count);
      float mults[number_count];
    };

    struct context {
      // тут должно храниться вообще все
      // аллокатор для окон, текущее окно, все шрифты, буферы
      // нет, шрифты будут храниться не здесь, шрифты это скорее всего отдельная сущность
      // так то не сильно соприкасается с гуишкой (где нам по большому счету плевать что такое контент)
      // тут еще будут заданы значение для приведения unit к абсолютным значениям

      // text_width, text_height, draw_frame - функции
      // style
      // size_t hover_id, focus_id, last_id (?)
      // last_rect, last_zindex, updated_focus (?), frame
      // hover_root, next_hover_root, scroll_target
      // number_edit_buf, number_edit
      // CONTAINERS, state pools (?)
      // mouse_pos, last_mouse_pos, mouse_delta, scroll_delta
      // mouse_down, mouse_pressed, key_down, key_pressed,
      // input_text

      unit_container unit;

      utils::arena_allocator window_data;
      utils::block_allocator window_pool;
      window_t* windows;
      window_t* current;

      // наверное практически для всей либы достаточно будет одного большого arena_allocator'а
      // окна скорее всего будут оставаться в памяти, стак виджетов будет активен только один
      // виджеты будут постепенно удаляться из памяти начиная с верхнего
      // то есть буквально не имеет особо смысла выделять в каждом окне отдельно память под виджеты
      // возможно вторая арена поменьше нужна для того чтобы добавить команды к отрисовке
      // + еще одна арена для шрифтов

      context();

      window_t* next_window(window_t* prev) const;
    };

    // вот мы создали это дело в контексте, что потом
    // потом нужно сгенерировать вершины/индексы + ножницы
    // что находится в функции end?


  }
}

#endif
