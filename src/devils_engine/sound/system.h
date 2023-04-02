#ifndef DEVILS_ENGINE_SOUND_SYSTEM_H
#define DEVILS_ENGINE_SOUND_SYSTEM_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>

#include "sound/decoder.h"
#include "utils/memory_pool.h"

typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;

//const size_t maximum_decoder_memory = 16096;

namespace devils_engine {
  namespace sound {
    struct settings {
      uint32_t type;
      float speed;
      float volume;
      float rnd_pitch;
      //float rnd_rate;
      // тут добавится источник положения звука + положение звука по умолчанию + еще несколько настроек

      settings();
      settings(
        const uint32_t type,
        const float speed = 1.0f,
        const float volume = 1.0f,
        const float rnd_pitch = 0.0f
      );
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

      system();
      ~system();

      system(const system &) noexcept = delete;
      system(system &&) noexcept = default;
      system & operator=(const system &) noexcept = delete;
      system & operator=(system &&) noexcept = default;

      size_t play_sound(const std::string_view &name, const uint32_t type, const float speed = 1.0f, const float volume = 1.0f);
      size_t play_sound(const std::string_view &name, const settings &info = settings());
      void stop_sound(const size_t id);

      void update(const size_t time);

      void load_resource(std::string id, const enum resource::type type, std::vector<char> buffer);

      size_t available_sources_count() const;
    private:
      struct current_playing_data {
        struct source source;
        struct settings info;
        const resource* res;
        size_t time;
        size_t loaded_frames;
        size_t id;

        void init(const size_t frames_count);
        void update_buffers(const size_t frames_count);
        size_t load_next(const uint32_t buffer, const size_t frames_count, const uint16_t channels);
      };

      ALCdevice* device;
      ALCcontext* ctx;
      size_t counter;

      std::vector<source> sources;
      std::vector<current_playing_data> current_sounds;

      utils::memory_pool<resource, sizeof(resource)*100> resource_pool;
      std::unordered_map<std::string_view, resource*> resources;
    };
  }
}

#endif
