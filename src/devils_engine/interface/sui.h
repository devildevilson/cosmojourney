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

#include "units.h"

#define SUI_INLINE inline

#define SUI_EPSILON 0.000001

// наверное все структуры лучше назвать с окончанием _t, чтобы путаницы не было

namespace devils_engine {
  // SuppleUI
  namespace sui {
    struct context;

    void end(context* ctx);

    struct raii_elem {
      context* ctx;
      bool value;

      constexpr raii_elem(context* ctx, const bool value = true) noexcept : ctx(ctx), value(value) {}
      SUI_INLINE ~raii_elem() noexcept { if (ctx != nullptr) end(ctx); }

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

    // все эти энумы мы можем смоделировать задав особый extent
    // нет, relative_to останется
    enum class relative_to {
      top_left,
      top_center,
      top_right,
      middle_left,
      middle_center,
      middle_right,
      bottom_left,
      bottom_center,
      bottom_right,
      count
    };
    
    // хотя может и не надо добавлять _t, в других библотечках нет такого
    struct handle {
      constexpr static const size_t underlying_size = 16;

      // нужен ли тип?
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

      SUI_INLINE constexpr vec2() noexcept : x(0.0f), y(0.0f) {}
      SUI_INLINE constexpr vec2(const float x, const float y) noexcept : x(x), y(y) {}
      SUI_INLINE ~vec2() noexcept = default;

      SUI_INLINE constexpr vec2(const vec2 &v) noexcept = default;
      SUI_INLINE constexpr vec2(vec2 &&v) noexcept = default;
      SUI_INLINE constexpr vec2 & operator=(const vec2 &v) noexcept = default;
      SUI_INLINE constexpr vec2 & operator=(vec2 &&v) noexcept = default;

      SUI_INLINE const float & operator[] (const size_t i) const noexcept { return i == 0 ? x : y; }
      SUI_INLINE float & operator[] (const size_t i) noexcept { return i == 0 ? x : y; }

      SUI_INLINE constexpr size_t length() const noexcept { return 2; }

      SUI_INLINE constexpr vec2 & operator+=(const vec2 &v) noexcept { x += v.x; y += v.y; return *this; }
      SUI_INLINE constexpr vec2 & operator-=(const vec2 &v) noexcept { x -= v.x; y -= v.y; return *this; }
      SUI_INLINE constexpr vec2 & operator*=(const vec2 &v) noexcept { x *= v.x; y *= v.y; return *this; }
      SUI_INLINE constexpr vec2 & operator/=(const vec2 &v) noexcept { x /= v.x; y /= v.y; return *this; }

      SUI_INLINE constexpr vec2 & operator+=(const float v) noexcept { x += v; y += v; return *this; }
      SUI_INLINE constexpr vec2 & operator-=(const float v) noexcept { x -= v; y -= v; return *this; }
      SUI_INLINE constexpr vec2 & operator*=(const float v) noexcept { x *= v; y *= v; return *this; }
      SUI_INLINE constexpr vec2 & operator/=(const float v) noexcept { x /= v; y /= v; return *this; }
    };

    SUI_INLINE constexpr vec2 operator+(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x + b.x, a.y + b.y); }
    SUI_INLINE constexpr vec2 operator-(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x - b.x, a.y - b.y); }
    SUI_INLINE constexpr vec2 operator*(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x * b.x, a.y * b.y); }
    SUI_INLINE constexpr vec2 operator/(const vec2 &a, const vec2 &b) noexcept { return vec2(a.x / b.x, a.y / b.y); }

    SUI_INLINE constexpr vec2 operator+(const vec2 &a, const float b) noexcept { return vec2(a.x + b, a.y + b); }
    SUI_INLINE constexpr vec2 operator-(const vec2 &a, const float b) noexcept { return vec2(a.x - b, a.y - b); }
    SUI_INLINE constexpr vec2 operator*(const vec2 &a, const float b) noexcept { return vec2(a.x * b, a.y * b); }
    SUI_INLINE constexpr vec2 operator/(const vec2 &a, const float b) noexcept { return vec2(a.x / b, a.y / b); }

    SUI_INLINE constexpr vec2 operator+(const float a, const vec2 &b) noexcept { return vec2(a + b.x, a + b.y); }
    SUI_INLINE constexpr vec2 operator-(const float a, const vec2 &b) noexcept { return vec2(a - b.x, a - b.y); }
    SUI_INLINE constexpr vec2 operator*(const float a, const vec2 &b) noexcept { return vec2(a * b.x, a * b.y); }
    SUI_INLINE constexpr vec2 operator/(const float a, const vec2 &b) noexcept { return vec2(a / b.x, a / b.y); }

    SUI_INLINE constexpr vec2 operator-(const vec2 &a) noexcept { return vec2(-a.x, -a.y); }
    SUI_INLINE constexpr vec2 abs(const vec2 &a) noexcept { return vec2(std::abs(a.x), std::abs(a.y)); }
    SUI_INLINE constexpr vec2 max(const vec2 &a, const vec2 &b) noexcept { return vec2(std::max(a.x, b.x), std::max(a.y, b.y)); }
    SUI_INLINE constexpr vec2 min(const vec2 &a, const vec2 &b) noexcept { return vec2(std::min(a.x, b.x), std::min(a.y, b.y)); }

    //SUI_INLINE constexpr bool operator==(const vec2 &a, const vec2 &b) noexcept { return a.x == b.x && a.y == b.y; }
    //SUI_INLINE constexpr bool operator!=(const vec2 &a, const vec2 &b) noexcept { return !operator==(a,b); }

    struct extent {
      unit w, h;

      constexpr extent() noexcept = default;
      constexpr extent(const float w, const float h) noexcept : w(w), h(h) {}
      constexpr extent(const unit &w, const unit &h) noexcept : w(w), h(h) {}
    };

    // сделал юниты, скорее всего юнитам нужно убрать конструктор только по float
    struct offset {
      unit x, y;

      constexpr offset() noexcept = default;
      constexpr offset(const float x, const float y) noexcept : x(x), y(y) {}
      constexpr offset(const unit &x, const unit &y) noexcept : x(x), y(y) {}
    };
    
    // основной класс для пользователя должен быть этот
    struct rect {
      struct offset offset;
      struct extent extent;

      constexpr rect() noexcept = default;
      constexpr rect(const struct extent &ex) noexcept : offset(0.0f, 0.0f), extent(ex) {}
      constexpr rect(const struct offset &of, const struct extent &ex) noexcept : offset(of), extent(ex) {}
    };

    struct abs_rect {
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
      constexpr abs_rect() noexcept = default;
      //rect(const vec2 &of);
      //rect(const vec2 &ex);
      constexpr abs_rect(const vec2 &of, const vec2 &ex) noexcept : offset(of), extent(ex) {}
      // const relative_to alignment = relative_to::top_left

      constexpr bool intersect(const vec2 &v) const noexcept {
        return contain(v);
      }

      constexpr bool contain(const vec2 &v) const noexcept {
        const auto to_space = v - offset;
        return to_space.x >= 0 && to_space.x <= extent.x &&
               to_space.y >= 0 && to_space.y <= extent.y;
      }

      constexpr bool intersect(const abs_rect &r) const noexcept {
        const auto center_point_to_space = abs((offset + extent / 2) - (r.offset + r.extent / 2)) * 2;
        const auto ext = extent + r.extent;
        return center_point_to_space.x <= ext.x &&
               center_point_to_space.y <= ext.y;
      }

      constexpr bool contain(const abs_rect &r) const noexcept {
        return contain(r.offset) && contain(r.offset+r.extent);
      }

      constexpr abs_rect extent_to_contain(const abs_rect &r) const noexcept {
        const auto min_p = min(offset, r.offset);
        return abs_rect(min_p, max(offset+extent, r.offset+r.extent) - min_p);
      }

      constexpr abs_rect shrink_to_intersection(const abs_rect &r) const noexcept {
        const auto min_p = max(offset, r.offset);
        return abs_rect(min_p, max(min(offset+extent, r.offset+r.extent) - min_p, vec2(0,0)));
      }

      constexpr vec2 make_extent(const abs_rect &next) noexcept {
        auto ext = next.extent;
        if (std::abs(ext.x) < SUI_EPSILON) { ext.x = extent.x - offset.x; }
        if (std::abs(ext.y) < SUI_EPSILON) { ext.y = extent.y - offset.y; }
        if (ext.x < 0) { ext.x += extent.x - offset.x; }
        if (ext.y < 0) { ext.y += extent.y - offset.y; }
        return ext;
      }

      // посчитаем бокс относительно этого
      constexpr abs_rect compute_absolute(const abs_rect &r) {
        const auto offset = offset+r.offset;
        const auto ext = make_extent(r);
        return abs_rect(offset, ext);
      }
    };

    static_assert(abs_rect(vec2(-1,-1), vec2(6,6)).intersect(abs_rect(vec2(5,5), vec2(2,2))));
    static_assert(abs_rect(vec2(-1,-1), vec2(6,6)).contain(abs_rect(vec2(-1,-1), vec2(6,6))));
    static_assert(abs_rect(vec2(-1,-1), vec2(6,6)).extent_to_contain(abs_rect(vec2(-2,-2), vec2(6,6))).contain(abs_rect(vec2(-2,-2), vec2(7,7))));

    // нам 250% потребуется взять часть изображения, в наклире использовались 2байта переменные
    // в микроюи вообще не использовались картинки, структура уже занимает 48 байт
    struct image {
      struct handle handle;
      vec2 size;
      rect region;

      constexpr image() noexcept = default;
      constexpr image(const struct handle &handle) noexcept : handle(handle) {}
      constexpr image(const struct handle &handle, const vec2 &size, const rect &region) noexcept :
        handle(handle), size(size), region(region)
      {}
    };

    //void init_context(context* ctx);

    // наверное тут нужно будет откидывать дробную часть
    abs_rect compute_absolute(const context* ctx, const rect &r);

    // точка входа для гуи - просто бокс с дополнительными настройками
    // имеет смысл тут вместо названия задать просто хеш, и функция называться должна по другому
    raii_elem window(context* ctx, const std::string_view &name, const rect &bounds, const relative_to content = relative_to::top_left);
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
    raii_elem box(context* ctx, const abs_rect &bounds = rect(), const relative_to content = relative_to::top_left);
    raii_elem box(context* ctx, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    // позволит сохранить текущее положение бокса контента, bounds - это стартовый размер, потом он перезапишется
    raii_elem box(context* ctx, const size_t hash, const abs_rect &bounds = rect(), const relative_to content = relative_to::top_left);
    raii_elem box(context* ctx, const size_t hash, const rect &bounds = rect(), const relative_to content = relative_to::top_left);
    // + боксы должны уметь быстро переопределять стиль отображения виджетов
    // как быть со стилями? стили по идее представляют из себя набор данных
    // которые мы используем чтобы не делать какие то часто повторяющиеся действия
    // например: есть 3 состояния бокса - активный, наведенный и нажатый (несколько нажатых?)
    // мы можем получить конкретное состояние бокса и нарисовать определенный цвет заливки
    // а можем сказать что у нас есть класс боксов у которых эти свойства похожи
    // просто возьми цвета и картинки вот отсюда

    // тот же бокс но который не сохраняет стейт и не реагирует на инпут
    // наверное по итогу не нужно, инпут будет иерархическим - единственная беда: кнопка в кнопке
    // я бы сказал что кнопка в кнопке - скорее неудачная идея
    //raii_elem_t group(context* ctx, const rect &bounds, const relative_to content = relative_to::top_left);

    // ожидаем последним аргументом веса будущих виджетов
    // эти функции - это не сами боксы это только бокс лайауты, тогда тут размер не нужен
    // эти функции должны последовательно ставить следующие виджеты друг за другом
    // увеличивая content_bounds (скорее всего content_bounds должен увеличиваться автоматически при расчете размеров)
    // вообще нам необязательно указывать размеры, мы получим их в любом случае
    // если по весам то по идее неважно какой знак
    raii_elem row(context* ctx, const rect &bounds, const std::span<float> &templ); // веса
    raii_elem row(context* ctx, const rect &bounds, const std::span<unit> &templ);  // конкретный размер
    raii_elem row(context* ctx, const rect &bounds); // ставим друг за другом
    
    raii_elem column(context* ctx, const rect &bounds, const std::span<float> &templ); // веса
    raii_elem column(context* ctx, const rect &bounds, const std::span<unit> &templ);  // конкретный размер
    raii_elem column(context* ctx, const rect &bounds); // ставим друг за другом

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
    raii_elem nine_slice(context* ctx, const abs_rect &bounds, const abs_rect &inner_bounds);
    raii_elem nine_slice(context* ctx, const rect &bounds, const rect &inner_bounds);

    // когда мы делаем функции row, column, nineslice ожидаем ли мы что функции
    // text, image, color будут занимать один слот? или все таки нужно оборачивать в бокс?
    // я бы предположил что второе, то есть чтобы продвинуть счетчик виджета нужно вызвать обязательно функцию бокс
    // а контент (text, image, color) - это чисто контент и он будет использовать предыдущие значения для отрисовки себя

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

    // контент строго берет только последний бокс? или получает свой? наверное первое
    enum class content_scale {
      fit,
      crop,
      fill_width,
      fill_height,
      fill_bounds,
      inside,
      none,
      count
    };
    void decorate(context* ctx, const image &img);
    void decorate(context* ctx, const handle &img, const vec2 &img_size, const content_scale scale = content_scale::fit);

    struct color_t {
      constexpr static const arr_size = sizeof(uint32_t);
      union {
        struct {
          uint8_t r,g,b,a;
        };
        uint8_t arr[arr_size];
        uint32_t container;
      };

      explicit color_t(const float r, const float g, const float b, const float a = 1.0f) noexcept;
      explicit color_t(const int r = 0, const int g = 0, const int b = 0, const int a = 255) noexcept;
    };
    void fill(context* ctx, const color_t &c); // заливка бокса цветом

    // размер определяется длиной строки и размером шрифта + нужно ли переводить на другую строку
    // определенно должен быть текст который займет ровно столько места сколько отведено
    // короче говоря чтобы враппинг сделать нормальный в любом случае нужна помощь со стороны боксов
    // я должен создать дополнительный бокс по высоте и ширине текста (или слова в тексте)
    // + алигнмент этих боксов
    // текст пытаемся впихнуть в тот бокс который предоставили
    template <typename... Args>
    void textf(context* ctx, const std::string_view &format, Args&&... args) {
      // нужно fmt пока что подключить
    }

    // должен быть некий базовый стиль для просто текста и заднего фона
    void text(context* ctx, const std::string_view &text);

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

    // размеры которые мы сюда передаем должны быть относительно предыдущего виджета
    // когда мы хотим расчитать конечный квадрат, мы расчитываем начиная с самого первого виджета (по вложенности)
    // проходим по иерархии и считаем поди compute_next
    // в микроюи данные лэйаута предыдущего виджета наследуются
    // по идее лэйаут не должен знать о том где находится и размер контента
    // при добавлении bounds мы можем добавить прокрутку, контент баундс тут ни к чему
    // короч нужно тогда форсить пользователя всегда задавать размеры (хотя мож и не обязательно)
    // точнее так: боксу можно не задавать а всем остальным обязательно нужно задать размеры
    // возможно размеры должны быть заданы в формате density_independent
    // если боксу не задаем размеры явно, то в зависимости от realtive_to бокс получит часть размера родителя
    // по крайней мере офсет (а размер че? по идее по контенту, ай проще просто задать по родительскому размеру)
    // короч предыдущий виджет должен точно знать свои размеры, значит бокс с дефолтными значениями должен 
    // занимать весь размер, в других билиотечках сразу заданы несколько виджетов по умолчанию
    // которые только вызывают compute_next не создавая новых лэйаутов, что делать с относительными размерами?
    // посчитать их заранее просто да и все
    class layout_i {
    public:
      size_t counter;
      abs_rect bounds;
      abs_rect content_bounds;
      abs_rect last_rect;
      //vec2 size;
      //vec2 offset;
      //vec2 max_size;

      SUI_INLINE layout_i(const abs_rect &bounds) noexcept :
        counter(0), bounds(bounds), content_bounds(bounds) // size(size),
      {}

      virtual ~layout_i() noexcept = default;
      virtual abs_rect compute_next(const abs_rect &next) = 0; // должен возвращать абсолютные размеры
      // возможно было бы неплохо превратить относительные размеры в абсолютные тоже тут
      //virtual abs_rect make_absolute(const rect &r) const = 0; // ctx?
      //virtual abs_rect compute_scissor() const = 0;
      inline bool within_bounds() const noexcept { return bounds.contain(content_bounds); }
    };

    class layout_default : public layout_i {
    public:
      relative_to content;

      layout_default(const abs_rect &bounds, const relative_to content) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
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
      //rect elem_bounds;

      layout_row(const abs_rect &bounds, const std::span<float> &weights) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
    };

    class layout_column : public layout_i {
    public:
      constexpr static const size_t maximum_weights = 32;

      float weights_summ;
      size_t count;
      std::array<float, maximum_weights> weights; // спан не сработает, нужна отдельная память 

      layout_column(const abs_rect &bounds, const std::span<float> &weights) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
    };

    class layout_row_dyn : public layout_i {
    public:
      float weights_summ;
      std::vector<float> weights;

      layout_row_dyn(const abs_rect &bounds, const std::span<float> &weights) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
    };

    class layout_column_dyn : public layout_i {
    public:
      float weights_summ;
      std::vector<float> weights;

      layout_column_dyn(const abs_rect &bounds, const std::span<float> &weights) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
    };

    class layout_nine_slice : public layout_i {
    public:
      rect inner_bounds;

      layout_nine_slice(const abs_rect &bounds, const abs_rect &inner_bounds) noexcept;
      abs_rect compute_next(const abs_rect &next) override;
    };

    // либо этот оставить либо убрать в пользу widget_data_t
    struct container_t {
      rect bounds;
      rect content_bounds;
      vec2 content_size; // обновляется как раз когда мы заканчиваем виджет
      vec2 scroll;
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

    struct draw_list {

    };

    class command_base : 
      // может быть ring ?
      public utils::forw::list<command_base, 0> 
    {
    public:
      //struct handle handle; // хендл тут?

      virtual ~command_base() noexcept = default;
      virtual void compute(draw_list* l) const = 0; // конст?
      inline command_base* next() const noexcept { return utils::forw::list_next<0>(this); };
      //inline command_base* next(const command_base* ref) const noexcept { return utils::ring::list_next<0>(this, ref); };
    };

    // ножницы добавляются только в разного рода панелях, с учетом того что мы должны знать размер панели заранее
    // ножницы мы можем добавить на этапе создания панели (собственно чаще всего так и происходит)
    class command_scissor : public command_base {
    public:
      abs_rect size;
      command_scissor(const abs_rect &size) noexcept;
      void compute(draw_list* l) const override;
    };

    class command_square : public command_base {
    public:
      //float rounding; // наверное проще сделать через отдельную команду отрисовки скругленного угла
      //float line_thickness; // проще сделать через отрисовку 9slice
      abs_rect size;
      struct color color;

      command_square(const abs_rect &size, const struct color &color) noexcept;
      // составим 2 треугольника для залитого квадрата
      // наверное в драв лист нужно указать что мы рисуем именно отдельными треугольниками
      void compute(draw_list* l) const override;
    };

    class command_round_corner : public command_square {
    public:
      // по идее ничего больше не нужно
      command_round_corner(const abs_rect &size, const struct color &color) noexcept;
      void compute(draw_list* l) const override;
    };

    class command_image : public command_square {
    public:
      struct image img;

      command_image(const abs_rect &size, const struct color &color, const struct image &img) noexcept;
      void compute(draw_list* l) const override;
    };

    class command_text : public command_base {
    public:
      const struct user_font *font;
      abs_rect size;
      std::string_view str; // место для строки возьмем сразу после этого класса

      command_text(const struct user_font *font, const abs_rect &size, const std::string_view &str) noexcept;
      void compute(draw_list* l) const override;
    };

    // наверное над лэйаутом будет еще контейнер, который может хранить данные между кадрами
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

      widget_data_t(widget_data_t* prev, layout_i* layout) noexcept;
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
      //widget_data_t* create_new_widget(const rect &bounds, layout_i* l) noexcept;
    };

    window_t* next_window(window_t* prev, window_t* start) const noexcept;

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

    // имеет ли смысл делать command_buffer для каждого окна?
    struct command_buffer {
      struct command_base *head, *tail;
      struct abs_rect clip;
      int use_clipping;
      handle userdata; // ??
    };
    
    // стиль должен быть расширяемый (должен содержать стили по типам)
    // как сделать? сопоставить типу индекс? необходимо сделать эту штуку уникальной для контекста
    struct style {
      template <typename T>
      static size_t get_type_id() noexcept;
      static size_t type_id;

      struct basic_style basic_style;
      utils::arena_allocator style_allocator;
      std::array<style_header*, 128> styles;

      template <typename T>
      size_t setup_style(const T& obj);
      template <typename T>
      T* get_style() const noexcept;
      style_header* get_style(const size_t index) const noexcept;
    };

    struct style_header {
      size_t type;
    };

    struct basic_style {
      const struct font* font;
      const struct color text_color;
      const struct color background_color;
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

      // какие размеры? может быть имеет смысл сделать компил тайм данные
      // но вообще window_data может хранить вообще все
      // где хранить clip rect? + где аллокатор для комманд? где хранить неважно: нужны указатели в окне
      // нужно где то хранить дополнительные данные виджетов: например текущую прокрутку
      // где? насколько большие эти данные?
      utils::arena_allocator command_allocator;
      utils::arena_allocator widget_allocator;
      utils::block_allocator window_pool;
      window_t* windows;
      window_t* current;
      struct command_buffer commands;

      // наверное практически для всей либы достаточно будет одного большого arena_allocator'а
      // окна скорее всего будут оставаться в памяти, стак виджетов будет активен только один
      // виджеты будут постепенно удаляться из памяти начиная с верхнего
      // то есть буквально не имеет особо смысла выделять в каждом окне отдельно память под виджеты
      // возможно вторая арена поменьше нужна для того чтобы добавить команды к отрисовке
      // + еще одна арена для шрифтов

      context();

      window_t* find_window(const size_t hash) const noexcept;
      window_t* create_window(const size_t hash, const std::string_view &str) noexcept;
      void remove_window(window_t* w) noexcept;
    };

    // вот мы создали это дело в контексте, что потом
    // потом нужно сгенерировать вершины/индексы + ножницы
    // что находится в функции end?


  }
}

#endif
