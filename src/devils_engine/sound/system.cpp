#include "system.h"

#include <iostream>
#include <cassert>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "al_helper.h"

#include "sound/mp3_decoder.h"
#include "sound/ogg_decoder.h"
#include "sound/wav_decoder.h"
#include "sound/flac_decoder.h"
#include "sound/pcm_decoder.h"
#include "utils/core.h"

#define SOUND_LOADING_COEFFICIENT 5.0f

namespace devils_engine {
  namespace sound {
    system::system() : device(nullptr), ctx(nullptr), counter(1) {
      ALCenum error = AL_NO_ERROR;

      device = alc_call(alcOpenDevice, nullptr, nullptr);
      assert(device != nullptr);

      const auto actual_device_name = alc_call(alcGetString, device, ALC_DEVICE_SPECIFIER);
      spdlog::info("Using sound output device {}", actual_device_name);

      ctx = alc_call(alcCreateContext, device, nullptr);

      if (!alc_call(alcMakeContextCurrent, device, ctx)) {
        spdlog::error("Could not make context current");
        throw std::runtime_error("OpenAL error");
      }

      al_call_info(alDistanceModel, AL_LINEAR_DISTANCE_CLAMPED);

      // создадим сорсы + 1 для музыки

      while (error == AL_NO_ERROR) {
        source s;

        alGenBuffers(2, s.buffers);
        error = alGetError();

        alGenSources(1, &s.handle);
        error = error != AL_NO_ERROR ? error : alGetError();

        sources.push_back(s);
      }

      if (sources.size() == 0 || (sources.size() == 1 && sources[0].handle == 0)) {
        check_al_error(error);
      }

      alDeleteBuffers(2, sources.back().buffers);
      sources.pop_back();

      spdlog::info("Created {} sound sources", sources.size());
    }

    system::~system() {
      for (const auto &source : sources) {
        alDeleteBuffers(2, source.buffers);
        alDeleteSources(1, &source.handle);
      }

      alc_call_info(alcMakeContextCurrent, device, nullptr);
      alc_call(alcDestroyContext, device, ctx);
      alc_call(alcCloseDevice, device);

      for (const auto& [ name, res ] : resources) {
        resource_pool.destroy(res);
      }
    }

    settings::settings() : settings(0) {}
    settings::settings(
      const uint32_t type,
      const float speed,
      const float volume,
      const float rnd_pitch
    ) : type(type), speed(std::max(0.0f, speed)), volume(std::clamp(volume, 0.0f, 1.0f)), rnd_pitch(rnd_pitch) {}

    size_t system::play_sound(const std::string_view &name, const uint32_t type, const float speed, const float volume) {
      return play_sound(name, settings(type, speed, volume));
    }

    size_t system::play_sound(const std::string_view &name, const settings &info) {
      // нам может потребоваться отслеживать состояние звука (например чтобы сменить положение звука)
      // как это сделать? можно вернуть указатель, но как проверить что звук все?
      // нужно чтобы с того конца вернули указатель
      // как понять какой звук нужно воспроизвести? строковый id, почему звуки будут храниться тут?
      // с ресурсами у меня пока что беда, не понимаю как лучше сделать, вообще в идеале
      // прямо сюда передавать собственно ресурс который мы хотим воспроизвести
      // лан лучше не выпендриваться и все таки сделать по имени
      // искать ресурс лучше прям здесь + прям здесь лучше всего ставить в очередь
      const auto itr = resources.find(name);
      if (itr == resources.end()) utils::error("Could not find resource {}", name);

      // по идее вот эту часть нужно закрыть за мьютексом
      auto source = std::move(sources.back());
      sources.pop_back();
      const size_t current_id = counter;
      counter += 1;
      counter += size_t(counter == 0);
      current_sounds.push_back(current_playing_data{source, info, itr->second, 0, 0, current_id});


      // да но нужно тогда сразу куда то это пихать, в queue?

      // нужно найти ресурс, взять свободный сорс, засунуть данные в очередь
      // + способ передачи позиции и направления звука
      // ничего лучше чем передавать указатель на класс с виртуальными методами мне в голову не приходит
      // если отсутствие ресурса - это фатал эррор то мы можем и позже его обработать

      //spdlog::info("Start playing {}", name);

      return current_id;
    }

    void system::stop_sound(const size_t id) {
      int64_t counter = int64_t(current_sounds.size())-1;
      while (counter >= 0) {
        auto &cur = current_sounds[counter];
        counter -= 1;

        if (cur.id != id) continue;

        al_call(alSourceStop, cur.source.handle);
        uint32_t buffers[2];
        al_call(alSourceUnqueueBuffers, cur.source.handle, 2, buffers);

        sources.emplace_back(cur.source);
        std::swap(current_sounds[counter], current_sounds.back());
        current_sounds.pop_back();
      }
    }

    // к сожалению мы работаем с объектами из массива по значению
    // поэтому придется вешать мьютекст на весь апдейт
    void system::update(const size_t time) {
      int64_t counter = int64_t(current_sounds.size())-1; // мьютекс
      while (counter >= 0) {
        auto &cur = current_sounds[counter]; // мьютекс + копирование (по идее ничего тяжелого)
        counter -= 1;

        const auto &snd_res = cur.res->sound;
        const size_t frames_to_load = second_to_pcm_frames_mono(SOUND_LOADING_COEFFICIENT, snd_res->sample_rate());
        cur.init(frames_to_load);
        cur.update_buffers(frames_to_load);
        cur.time += time;
        // обновляем громкость

        ALint state = AL_PLAYING;
        al_call(alGetSourcei, cur.source.handle, AL_SOURCE_STATE, &state);

        if (state == AL_PLAYING) continue;

        // удаляем звук который перестал играть
        spdlog::info("Stop playing {}", cur.res->id);
        uint32_t buffers[2] = {0,0};
        al_call(alSourceUnqueueBuffers, cur.source.handle, 2, buffers);

        // мьютекс
        //al_call(alSourcei, cur.source.handle, AL_SOURCE_STATE, );
        sources.emplace_back(cur.source);
        std::swap(current_sounds[counter], current_sounds.back());
        current_sounds.pop_back();
      }
    }

    void system::load_resource(std::string id, const enum resource::type type, std::vector<char> buffer) {
      const auto itr = resources.find(id);
      if (itr != resources.end()) utils::error("Resource {} is already created", std::string_view(id));

      auto res = resource_pool.create(std::move(id), type, std::move(buffer));
      resources.emplace(res->id, res);
    }

    size_t system::available_sources_count() const {
      return sources.size();
    }

    system::resource::resource() : type(type::undefined) {}
    system::resource::resource(std::string id, enum type type, std::vector<char> buffer) :
      id(std::move(id)),
      type(type),
      buffer(std::move(buffer))
    {
      if (type == type::mp3) {
        sound.reset(new  mp3_decoder(this->id, this->buffer.data(), this->buffer.size()));
      } else if (type == type::wav) {
        sound.reset(new  wav_decoder(this->id, this->buffer.data(), this->buffer.size()));
      } else if (type == type::ogg) {
        sound.reset(new  ogg_decoder(this->id, this->buffer.data(), this->buffer.size()));
      } else if (type == type::flac) {
        sound.reset(new flac_decoder(this->id, this->buffer.data(), this->buffer.size()));
      } else {
        utils::error("Invalid sound resource type {}", size_t(type));
      }

      const size_t frames_treshold = second_to_pcm_frames(SOUND_LOADING_COEFFICIENT, sound->sample_rate(), sound->channels());
      if (sound->frames_count() < frames_treshold) {
        auto dec = new pcm_decoder(sound.get());
        sound.reset(dec);
        this->type = type::pcm;
      }
    }

    system::resource::~resource() noexcept {
      //sound->~decoder();
    }

    void system::current_playing_data::init(const size_t frames_count) {
      if (time != 0) return;
      // максимальная скорость? должна быть по идее ограничена сверху количеством сэмплов в файле
      // бессмысленно менять sample_rate если есть pitch
      // я так понимаю в опенал без дополнительных прибамбасов это одно и тоже

      //spdlog::info("resource {} sample rate {}", std::string_view(res->id), sample_rate);

      // loaded_frames этой переменной наверное будет управлять полностью функция load_next
      loaded_frames += load_next(source.buffers[0], frames_count, 1);
      loaded_frames += load_next(source.buffers[1], frames_count, 1);
      al_call(alSourceQueueBuffers, source.handle, 2, source.buffers);
      al_call(alSourcef, source.handle, AL_GAIN, info.volume);
      al_call(alSourcef, source.handle, AL_PITCH, info.speed);
      al_call(alSourcePlay, source.handle);
    }

    void system::current_playing_data::update_buffers(const size_t frames_count) {
      int32_t processed_buffers_count = 0;
      al_call(alGetSourcei, source.handle, AL_BUFFERS_PROCESSED, &processed_buffers_count);
      //spdlog::info("frames_count {} processed_buffers_count {} time {}", frames_count, processed_buffers_count, time);
      if (processed_buffers_count == 0 || loaded_frames >= res->sound->frames_count()) return;

      uint32_t buffer = 0;
      al_call(alSourceUnqueueBuffers, source.handle, 1, &buffer);

      //const int64_t sample_rate = res->sound->sample_rate() * info.speed;
      //if (sample_rate <= 0) return;

      loaded_frames += load_next(buffer, frames_count, 1); // как залупить звук? + как залупить мелкий звук?

      al_call(alSourceQueueBuffers, source.handle, 1, &buffer);
    }

    size_t system::current_playing_data::load_next(
      const uint32_t buffer,
      const size_t frames_count,
      const uint16_t channels
    ) {
      // тут так не получится, нужно заполнять отдельный буфер, сейчас я буду перезаписывать данные в АЛ буфере
      // size_t local_loaded_frames = frames_count;
      // while (local_loaded_frames > 0) {
      //   if (!res->sound->seek(loaded_frames))
      //     utils::error("seek to pcm frame {} failed in resource '{}'", loaded_frames, res->id);
      //
      //   const size_t frames = res->sound->get_frames(buffer, local_loaded_frames, channels);
      //   local_loaded_frames -= frames;
      //
      //
      // }
      if (!res->sound->seek(loaded_frames))
        utils::error("seek to pcm frame {} failed in resource '{}'", loaded_frames, res->id);
      const size_t frames = res->sound->get_frames(buffer, frames_count, channels);
      // if (frames < frames_count) {
      //   // если фреймов загрузили меньше чем требовали И этот звук нужно зациклить
      //   // то нужно дозагрузить часть в буфер, если звук совсем маленький то может быть
      //   // несколько раз сделать эту операцию
      // }
      // loaded_frames += frames;
      return frames;
    }
  }
}
