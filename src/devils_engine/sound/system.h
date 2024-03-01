#ifndef DEVILS_ENGINE_SOUND_SYSTEM_H
#define DEVILS_ENGINE_SOUND_SYSTEM_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>

#include <glm/vec3.hpp>

#include "sound/decoder.h"
#include "utils/memory_pool.h"

typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;

//const size_t maximum_decoder_memory = 16096;

namespace devils_engine {
  namespace sound {
    // звуки автоматически убираем из системы когда заканчиваются данные
    // звук который хотим залупить не уберется пока мы не скажем системе об этом
    // нужно еще как то указать количество каналов в которое мы хотим упихать звук
    // для обычного типа - это почти всегда моно звук, для музыки - стерео
    // нужно ли когда нибудь подгружать звук в состоянии паузы? вряд ли, все что сюда попадает пытаемся сразу запустить
    struct settings {
      uint32_t type;
      float speed;
      float volume;
      float rnd_pitch;
      //float rnd_rate;
      // тут добавится источник положения звука + положение звука по умолчанию + еще несколько настроек
      // как понять что нужно звук залупить? скорее всего это со стороны источника звука... не совсем
      // как будто действительно нужно занять слот звука и ждать пока не придет команда остановить звук
      // он мне не нравится конечно
      bool is_loop;
      bool is_mono;
      bool is_needed; // если все источники сейчас заняты, то дропаем звук - подойдет для мелких звуков
      uint32_t force_source;
      // координаты
      glm::vec3 pos;
      glm::vec3 dir;
      glm::vec3 vel;

      settings() noexcept;

      settings(
        const uint32_t type,
        const float volume = 1.0f,
        const float speed = 1.0f,
        const float rnd_pitch = 0.0f,
        const bool is_mono = true
      ) noexcept;

      settings(
        const uint32_t type,
        const uint32_t force_source,
        const bool is_mono = false
      ) noexcept;
    };

    class system {
    public:
      using handle_t = uint32_t;

      struct source {
        handle_t handle;
        handle_t buffers[2];

        inline source() : handle(0), buffers{0, 0} {}
      };

      struct resource {
        enum class type {
          mp3,
          flac,
          wav,
          ogg,
          pcm,
          undefined
        };

        std::string id;
        enum type type;
        std::unique_ptr<sound::decoder> sound;
        //char decoder_memory[maximum_decoder_memory]; // для ресурса это необязательно, ресурс загрузим один раз и забудем про него
        // еще обязательно нужно хранить весь оригинальный файл в памяти
        std::vector<char> buffer;

        resource();
        // некоторые ресурсы нужно перевести сразу в pcm формат
        resource(std::string id, enum type type, std::vector<char> buffer);
        ~resource() noexcept;
      };

      system(const size_t queue_size = 3);
      ~system();

      system(const system &) noexcept = delete;
      system(system &&) noexcept = default;
      system & operator=(const system &) noexcept = delete;
      system & operator=(system &&) noexcept = default;

      // должны быть отдельные функции для музыки заднего фона, тут чуть все по другому (хотя может и не совсем)
      // нет как будто для заднего фона мы просто сделаем эксклюзивный источник (1 или 2)
      // и в настройки звука имеет смысл указать форсированный номер источника

      // что изменится? ресурсы хранить будем не тут а в другом месте (тут будем передавать собственно сам ресурс а не название)
      // добавится несколько функций для того чтобы следить за звуком и менять его местоположение
      // чтобы следить за звуком нужно возвращать какой то идентификатор
      // чтобы воспроизвести звук нам даже потребуется: ресурс, идентификатор источника + идентификатор звука
      // думаю что индентификатором звука имеет смысл сделать номер источника
      // не, думаю что лучше для id просто генерировать какое нибудь случайное число - тогда поиск будет сложнее
      // возможно нужно добавить полностью управление звуком типо пауза и продолжить
      //const size_t producer_id, 
      size_t setup_sound(const resource *res, const settings &info = settings());
      bool remove_sound(const size_t source_id);
      bool play_sound(const size_t source_id); // мы не хотим этим пользоваться нигде кроме музыки
      bool stop_sound(const size_t source_id); // мы не хотим этим пользоваться нигде кроме музыки (наверное хотим приостановить все звуки в меню)
      float stat_sound(const size_t source_id) const; // [0,1]
      bool set_sound(const size_t source_id, const float place); // [0,1]
      bool set_sound(const size_t source_id, const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel);

      bool set_listener_pos(const glm::vec3 &pos);
      bool set_listener_ori(const glm::vec3 &look_at, const glm::vec3 &up);
      bool set_listener_vel(const glm::vec3 &vel);

      void update(const size_t time);

      //void load_resource(std::string id, const enum resource::type type, std::vector<char> buffer);

      size_t available_sources_count() const;
    private:
      // немного поменяем структуру: у каждого источника очередь из 3-5 звуков
      // звуки кладем в пустую очередь, в апдейте проверяем очереди
      // некоторые звуки имеют явный приоритет над другими, тогда как будто срочно нужно поменять воспроизводимый звук
      // 
      struct current_playing_data {
        struct source source;
        struct settings info; // возможно не все настройки нам отсюда нужны
        const resource* res;
        size_t time;
        size_t loaded_frames; // по идее по количеству фреймов мы можем понять долю звука
        size_t id;

        void init(const size_t frames_count);
        void update_buffers(const size_t frames_count);
        size_t load_next(const uint32_t buffer, const size_t frames_count, const uint16_t channels);
      };

      struct sound_processing_data {
        struct settings info; // возможно не все настройки нам отсюда нужны
        const resource *res;
        size_t time;
        size_t loaded_frames;
        size_t id;

        sound_processing_data() noexcept;
        void init(const size_t id, const resource *res, const struct settings &info) noexcept;
        void reset() noexcept;
        size_t load_next(const uint32_t buffer, const size_t frames_count, const uint16_t channels);
      };

      struct source_data {
        struct source source;
        //std::array<sound_processing_data, 3> queue;
        sound_processing_data *queue;

        inline source_data() noexcept : queue(nullptr) {}
        source_data(const struct source &source, sound_processing_data *queue) noexcept;
        void init(const size_t frames_count);
        void update_buffers(const size_t frames_count);
      };

      ALCdevice* device;
      ALCcontext* ctx;
      size_t counter;
      size_t queue_size;
      size_t sources_offset;

      //std::vector<source> sources;
      //std::vector<current_playing_data> current_sounds;

      std::vector<source_data> sources;
      std::unique_ptr<sound_processing_data[]> proc_array;

      //utils::memory_pool<resource, sizeof(resource)*100> resource_pool;
      //std::unordered_map<std::string_view, resource*> resources;

      size_t get_new_id();
      std::tuple<size_t, size_t> find_source_id(const size_t source_id) const;
      void remove_from_queue(sound_processing_data *queue, const size_t index);
    };
  }
}

#endif
