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

#define SOUND_LOADING_COEFFICIENT 1.0f
#define SMALL_SOUND_COEFFICIENT 5.0f

namespace devils_engine {
  namespace sound {
    system::system(const size_t queue_size) : device(nullptr), ctx(nullptr), counter(1), queue_size(queue_size), sources_offset(1) {
      ALCenum error = AL_NO_ERROR;

      device = alc_call(alcOpenDevice, nullptr, nullptr);
      assert(device != nullptr);

      const auto actual_device_name = alc_call(alcGetString, device, ALC_DEVICE_SPECIFIER);
      utils::info("sound::system: Using sound output device {}", actual_device_name);

      ctx = alc_call(alcCreateContext, device, nullptr);

      if (!alc_call(alcMakeContextCurrent, device, ctx)) {
        utils::error("sound::system: OpenAL: Could not make context current");
      }

      al_call_info(alDistanceModel, AL_LINEAR_DISTANCE_CLAMPED);

      // создадим сорсы + 1 для музыки

      while (error == AL_NO_ERROR) {
        source s;

        alGenBuffers(2, s.buffers);
        error = alGetError();

        alGenSources(1, &s.handle);
        error = error != AL_NO_ERROR ? error : alGetError();

        sources.push_back(system::source_data(s, nullptr));
      }

      if (sources.size() == 0 || (sources.size() == 1 && sources[0].source.handle == 0)) {
        check_al_error(error);
      }

      alDeleteBuffers(2, sources.back().source.buffers);
      sources.pop_back();

      proc_array.reset(new sound_processing_data[sources.size() * queue_size]);

      for (size_t i = 0; i < sources.size(); ++i) {
        sources[i].queue = &proc_array[i*queue_size];
      }

      utils::info("sound::system: Created {} sound sources", sources.size());
    }

    system::~system() {
      for (const auto &data : sources) {
        alDeleteBuffers(2, data.source.buffers);
        alDeleteSources(1, &data.source.handle);
      }

      alc_call_info(alcMakeContextCurrent, device, nullptr);
      alc_call(alcDestroyContext, device, ctx);
      alc_call(alcCloseDevice, device);

      /*for (const auto& [ name, res ] : resources) {
        resource_pool.destroy(res);
      }*/
    }

    settings::settings() noexcept : settings(0,0, false) {}

    settings::settings(
      const uint32_t type,
      const float volume,
      const float speed,
      const float rnd_pitch,
      const bool is_mono
    ) noexcept : 
      type(type), speed(speed), volume(volume), rnd_pitch(rnd_pitch), 
      is_loop(false), is_mono(is_mono), is_needed(false), force_source(UINT32_MAX),
      pos(0.0f, 0.0f, 0.0f),
      dir(0.0f, 0.0f, 0.0f),
      vel(0.0f, 0.0f, 0.0f)
    {}

    settings::settings(
      const uint32_t type,
      const uint32_t force_source,
      const bool is_mono
    ) noexcept : 
      type(type), speed(1.0f), volume(1.0f), rnd_pitch(0.0f), 
      is_loop(false), is_mono(is_mono), is_needed(false), force_source(force_source),
      pos(0.0f, 0.0f, 0.0f),
      dir(0.0f, 0.0f, 0.0f),
      vel(0.0f, 0.0f, 0.0f)
    {}

    size_t system::setup_sound(const system::resource *res, const settings &info) {
      if (info.type >= volume_set::sound_types_count) utils::error("Could not set volume to sound type {} max is {}", info.type, volume_set::sound_types_count);

      if (info.force_source < sources.size()) {
        for (size_t i = 0; i < queue_size; ++i) {
          auto &data = sources[info.force_source];
          if (data.queue[i].id != 0) continue;

          const size_t sound_id = get_new_id();
          data.queue[i].init(sound_id, res, info);

          return sound_id;
        }

        return 0;
      }

      for (size_t i = 0; i < queue_size; ++i) {
        for (size_t j = sources_offset; j < sources.size(); ++j) {
          auto &data = sources[j];
          if (data.queue[i].id != 0) continue;

          const size_t sound_id = get_new_id();
          data.queue[i].init(sound_id, res, info);

          return sound_id;
        }
      }

      return 0;
    }

    bool system::remove_sound(const size_t source_id) {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return false;
      if (queue_index == 0) al_call(alSourceStop, sources[source_index].source.handle);

      // оставим всю основную работу апдейту?
      remove_from_queue(sources[source_index].queue, queue_index);

      return true;
    }

    bool system::play_sound(const size_t source_id) {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return false;

      ALint ret; 
      al_call(alGetSourcei, sources[source_index].source.handle, AL_SOURCE_STATE, &ret);
      if (ret == AL_PLAYING) return true;
      al_call(alSourcePlay, sources[source_index].source.handle);
      return true;
    }

    bool system::stop_sound(const size_t source_id) {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return false;
      if (queue_index != 0) return false; // паузим если только это первый звук в очереди

      al_call(alSourcePause, sources[source_index].source.handle);
      return true;
    }

    double system::stat_sound(const size_t source_id) const {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return -1.0;
      if (queue_index != 0) return 0.0;

      const auto cur = sources[source_index].queue;

      // текущая позиция звука расчитывается так:
      // загруженные семплы - буфер - буфер + текущая позиция семпла
      ALint ret;
      al_call(alGetSourcei, sources[source_index].source.handle, AL_SAMPLE_OFFSET, &ret);
      ALint buffer_bytes1;
      ALint buffer_bytes2;
      al_call(alGetBufferi, sources[source_index].source.buffers[0], AL_SIZE, &buffer_bytes1);
      al_call(alGetBufferi, sources[source_index].source.buffers[1], AL_SIZE, &buffer_bytes2);
      const uint32_t channels = cur->info.is_mono ? 1 : cur->res->sound->channels();
      const uint32_t bits = adjust_bits_per_channel(cur->res->sound->bits_per_channel());
      const size_t frames1 = bytes_to_pcm_frames(buffer_bytes1, channels, bits);
      const size_t frames2 = bytes_to_pcm_frames(buffer_bytes2, channels, bits);
      const size_t processed_samples = cur->loaded_frames - frames1 - frames2 + ret;

      return double(processed_samples) / double(cur->res->sound->frames_count());
    }

    bool system::set_sound(const size_t source_id, const double place) {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return -1.0;

      const auto cur = &sources[source_index].queue[queue_index];
      if (queue_index == 0) {
        al_call(alSourceStop, sources[source_index].source.handle);
        uint32_t buffers[2] = {0,0};
        al_call(alSourceUnqueueBuffers, sources[source_index].source.handle, 2, buffers);
      }

      cur->loaded_frames = place * cur->res->sound->frames_count();
      cur->time = 0;

      return false;
    }

    bool system::set_sound(const size_t source_id, const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) {
      const auto [source_index, queue_index] = find_source_id(source_id);
      if (source_index == SIZE_MAX) return false;

      auto &info = sources[source_index].queue[queue_index].info;
      info.pos = pos;
      info.dir = dir;
      info.vel = vel;

      if (queue_index == 0) {
        al_call(alSource3f, sources[source_index].source.handle, AL_POSITION, pos.x, pos.y, pos.z);
        al_call(alSource3f, sources[source_index].source.handle, AL_DIRECTION, dir.x, dir.y, dir.z);
        al_call(alSource3f, sources[source_index].source.handle, AL_VELOCITY, vel.x, vel.y, vel.z);
      }

      return true;
    }

    bool system::set_listener_pos(const glm::vec3 &pos) {
      al_call(alListener3f, AL_POSITION, pos.x, pos.y, pos.z);
      return true;
    }

    bool system::set_listener_ori(const glm::vec3 &look_at, const glm::vec3 &up) {
      const ALfloat listener_ori[6] = { look_at.x, look_at.y, look_at.z, up.x, up.y, up.z };
      al_call(alListenerfv, AL_ORIENTATION, listener_ori);
      return true;
    }

    bool system::set_listener_vel(const glm::vec3 &vel) {
      al_call(alListener3f, AL_VELOCITY, vel.x, vel.y, vel.z);
      return true;
    }

    void system::set_master_volume(const float val) {
      volume.master = std::clamp(val, 0.0f, 1.0f);
    }

    void system::set_source_volume(const uint32_t type, const float val) {
      if (type >= volume_set::sound_types_count) utils::error("Could not set volume to sound type {} max is {}", type, volume_set::sound_types_count);
      volume.source[type] = std::clamp(val, 0.0f, 1.0f);
    }

    // пройдемся по всем источникам, если что то закончило играть ставим следующий звук в очереди
    void system::update(const size_t time) {
      al_call(alListenerf, AL_GAIN, volume.master);

      for (auto &data : sources) {
        if (data.queue->id == 0) continue;

        const float source_volume = volume.source[data.queue[0].info.type];
        const auto &snd_res = data.queue[0].res->sound;
        //const uint16_t channels_count = data.queue->info.is_mono ? 1 : snd_res->channels();
        const size_t frames_to_load = second_to_pcm_frames(SOUND_LOADING_COEFFICIENT, snd_res->sample_rate());
        //utils::println("pcm frames", frames_to_load, SOUND_LOADING_COEFFICIENT, snd_res->sample_rate(), channels_count);
        data.init(source_volume, frames_to_load); // по идее во всех новых звуках time должен быть 0
        data.update(source_volume, frames_to_load);
        data.queue->time += time;

        // громкость зависит от типа звука, тип звука указываем в настройках

        ALint state = AL_PLAYING;
        al_call(alGetSourcei, data.source.handle, AL_SOURCE_STATE, &state);

        if (state == AL_PLAYING || state == AL_PAUSED) continue;

        utils::info("Stop playing {}", data.queue->res->id);
        uint32_t buffers[2] = {0,0};
        al_call(alSourceUnqueueBuffers, data.source.handle, 2, buffers);

        remove_from_queue(data.queue, 0);
      }
    }

    /*void system::load_resource(std::string id, const enum resource::type type, std::vector<char> buffer) {
      const auto itr = resources.find(id);
      if (itr != resources.end()) utils::error("Resource {} is already created", std::string_view(id));

      auto res = resource_pool.create(std::move(id), type, std::move(buffer));
      resources.emplace(res->id, res);
    }*/

    size_t system::available_sources_count() const {
      return sources.size();
    }

    size_t system::get_new_id() { 
      const size_t id = counter;
      counter += 1;
      counter += size_t(counter == 0);
      return id;
    }

    std::tuple<size_t, size_t> system::find_source_id(const size_t source_id) const {
      if (source_id == 0) return std::make_tuple(SIZE_MAX, SIZE_MAX);

      // вообще необязательно использовать sources
      for (size_t j = 0; j < sources.size(); ++j) {
        for (size_t i = 0; i < queue_size; ++i) {
          if (proc_array[j * queue_size + i].id != source_id) continue;

          return std::make_tuple(j, i);
        }
      }

      return std::make_tuple(SIZE_MAX, SIZE_MAX);
    }

    void system::remove_from_queue(system::sound_processing_data *queue, const size_t index) {
      if (index >= queue_size) return;

      for (size_t i = 0; i < queue_size-1; ++i) {
        queue[i] = queue[i + 1];
      }
      queue[queue_size - 1] = sound_processing_data();
    }

    system::volume_set::volume_set() noexcept
      : master(1.0f), source{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
    {}

    const size_t system::volume_set::sound_types_count;

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

      const size_t frames_treshold = second_to_pcm_frames(SMALL_SOUND_COEFFICIENT, sound->sample_rate()); // , sound->channels()
      if (sound->frames_count() < frames_treshold) {
        auto dec = new pcm_decoder(sound.get());
        sound.reset(dec);
        this->type = type::pcm;
      }
    }

    system::resource::~resource() noexcept {
      //sound->~decoder();
    }

    double system::resource::duration() const {
      return pcm_frames_to_seconds(sound->frames_count(), sound->sample_rate());
    }

    // void system::current_playing_data::init(const size_t frames_count) {
    //   if (time != 0) return;
    //   // максимальная скорость? должна быть по идее ограничена сверху количеством сэмплов в файле
    //   // бессмысленно менять sample_rate если есть pitch
    //   // я так понимаю в опенал без дополнительных прибамбасов это одно и тоже
    //
    //   //spdlog::info("resource {} sample rate {}", std::string_view(res->id), sample_rate);
    //
    //   // loaded_frames этой переменной наверное будет управлять полностью функция load_next
    //   loaded_frames += load_next(source.buffers[0], frames_count, 1);
    //   loaded_frames += load_next(source.buffers[1], frames_count, 1);
    //   al_call(alSourceQueueBuffers, source.handle, 2, source.buffers);
    //   al_call(alSourcef, source.handle, AL_GAIN, info.volume);
    //   al_call(alSourcef, source.handle, AL_PITCH, info.speed);
    //   al_call(alSourcePlay, source.handle);
    // }
    //
    // void system::current_playing_data::update_buffers(const size_t frames_count) {
    //   int32_t processed_buffers_count = 0;
    //   al_call(alGetSourcei, source.handle, AL_BUFFERS_PROCESSED, &processed_buffers_count);
    //   //spdlog::info("frames_count {} processed_buffers_count {} time {}", frames_count, processed_buffers_count, time);
    //   if (processed_buffers_count == 0 || loaded_frames >= res->sound->frames_count()) return;
    //
    //   uint32_t buffer = 0;
    //   al_call(alSourceUnqueueBuffers, source.handle, 1, &buffer);
    //
    //   //const int64_t sample_rate = res->sound->sample_rate() * info.speed;
    //   //if (sample_rate <= 0) return;
    //
    //   loaded_frames += load_next(buffer, frames_count, 1); // как залупить звук? + как залупить мелкий звук?
    //
    //   al_call(alSourceQueueBuffers, source.handle, 1, &buffer);
    // }
    //
    // size_t system::current_playing_data::load_next(
    //   const uint32_t buffer,
    //   const size_t frames_count,
    //   const uint16_t channels
    // ) {
    //   // тут так не получится, нужно заполнять отдельный буфер, сейчас я буду перезаписывать данные в АЛ буфере
    //   // size_t local_loaded_frames = frames_count;
    //   // while (local_loaded_frames > 0) {
    //   //   if (!res->sound->seek(loaded_frames))
    //   //     utils::error("seek to pcm frame {} failed in resource '{}'", loaded_frames, res->id);
    //   //
    //   //   const size_t frames = res->sound->get_frames(buffer, local_loaded_frames, channels);
    //   //   local_loaded_frames -= frames;
    //   //
    //   //
    //   // }
    //   if (!res->sound->seek(loaded_frames))
    //     utils::error("seek to pcm frame {} failed in resource '{}'", loaded_frames, res->id);
    //   const size_t frames = res->sound->get_frames(buffer, frames_count, channels);
    //   // if (frames < frames_count) {
    //   //   // если фреймов загрузили меньше чем требовали И этот звук нужно зациклить
    //   //   // то нужно дозагрузить часть в буфер, если звук совсем маленький то может быть
    //   //   // несколько раз сделать эту операцию
    //   // }
    //   // loaded_frames += frames;
    //   return frames;
    // }

    system::sound_processing_data::sound_processing_data() noexcept
      : res(nullptr), time(0), loaded_frames(0), id(0) {}

    void system::sound_processing_data::init(
        const size_t id, const resource *res,
        const struct settings &info
    ) noexcept {
      this->id = id;
      this->res = res;
      this->info = info;
      this->time = 0;
      this->loaded_frames = 0;
    }

    void system::sound_processing_data::reset() noexcept { 
      init(0, nullptr, settings());
    }

    size_t system::sound_processing_data::load_next(
      const uint32_t buffer,
      const size_t frames_count,
      const uint16_t channels
    ) {
      if (!res->sound->seek(loaded_frames))
        utils::error("seek to pcm frame {} failed in resource '{}'", loaded_frames, res->id);

      const uint16_t final_channels = std::min(channels, res->sound->channels());
      const size_t frames = res->sound->get_frames(buffer, frames_count, final_channels);

      return frames;
    }

    system::source_data::source_data(const struct source &source, sound_processing_data *queue) noexcept 
      : source(source), queue(queue)
    {}

    void system::source_data::init(const float volume, const size_t frames_count) {
      if (queue->time != 0) return;

      const uint16_t channels_count = queue->info.is_mono ? 1 : 0;
      queue->loaded_frames += queue->load_next(source.buffers[0], frames_count, channels_count);
      queue->loaded_frames += queue->load_next(source.buffers[1], frames_count, channels_count);
      al_call(alSourceQueueBuffers, source.handle, 2, source.buffers);
      al_call(alSourcef, source.handle, AL_GAIN, queue->info.volume);
      al_call(alSourcef, source.handle, AL_PITCH, queue->info.speed);

      const auto pos = queue->info.pos, dir = queue->info.dir, vel = queue->info.vel;
      al_call(alSource3f, source.handle, AL_POSITION, pos.x, pos.y, pos.z);
      al_call(alSource3f, source.handle, AL_DIRECTION, dir.x, dir.y, dir.z);
      al_call(alSource3f, source.handle, AL_VELOCITY, vel.x, vel.y, vel.z);

      const float gain = volume * std::clamp(queue->info.volume, 0.0f, 1.0f);
      al_call(alSourcef, source.handle, AL_GAIN, gain);

      al_call(alSourcePlay, source.handle);
    }

    void system::source_data::update(const float volume, const size_t frames_count) {
      // надо перенести в
      const float gain = volume * std::clamp(queue->info.volume, 0.0f, 1.0f);
      al_call(alSourcef, source.handle, AL_GAIN, gain);

      int32_t processed_buffers_count = 0;
      al_call(alGetSourcei, source.handle, AL_BUFFERS_PROCESSED, &processed_buffers_count);
      //spdlog::info("frames_count {} processed_buffers_count {} time {}", frames_count, processed_buffers_count, time);
      if (processed_buffers_count == 0) return;
      if (queue->loaded_frames >= queue->res->sound->frames_count() && !queue->info.is_loop) return;

      queue->loaded_frames = queue->loaded_frames >= queue->res->sound->frames_count() ? 0 : queue->loaded_frames;

      uint32_t buffer = 0;
      al_call(alSourceUnqueueBuffers, source.handle, 1, &buffer);

      const uint16_t channels_count = queue->info.is_mono ? 1 : 0;
      queue->loaded_frames += queue->load_next(buffer, frames_count, channels_count); // как залупить звук? + как залупить мелкий звук?

      al_call(alSourceQueueBuffers, source.handle, 1, &buffer);
    }
  }
}
