#include "basic_sources.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "al_helper.h"

namespace devils_engine {
namespace sound {

void background_source::set_resource(const resource* res) {
  basic_sound_data::res = res;
  set_stat(0.0);
}

bool background_source::is_valid() const {
  return basic_sound_data::res != nullptr;
}

const resource* background_source::currently_playing() const {
  return basic_sound_data::res;
}

void background_source::set(float , float volume, float , bool ) {
  basic_sound_data::volume = volume;
  if (sound_processing_data::fast_source == 0) return;

  const float final_vol = *basic_sound_data::type_volume * volume;
  al_call(alSourcef, sound_processing_data::fast_source, AL_GAIN, final_vol);
}

void background_source::set_transform(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &) {}

void background_source::set_relative(const bool relative) {
  if (sound_processing_data::fast_source != 0) {
    al_call(alSourcei, sound_processing_data::fast_source, AL_SOURCE_RELATIVE, int(relative));
  }
}

bool background_source::play() {
  if (!is_valid()) return false;
  if (is_playing()) return false;
  sound_processing_data::time = 0;
  return true;
}

bool background_source::pause() {
  if (!is_valid()) return false;
  sound_processing_data::time = SIZE_MAX;
  return true;
}

bool background_source::is_playing() const {
  return is_valid() && sound_processing_data::time != SIZE_MAX;
}

bool background_source::is_paused() const {
  return sound_processing_data::time == SIZE_MAX;
}

double background_source::stat() const {
  //if (sound_processing_data::fast_source == 0) return 0.0;
  if (!is_playing()) return 0.0;

  // может быть вариант что мы приостановим звук
  // и все равно захотим стат сделать
  // после приостановки уберем fast_source

  ALint samples_offset = 0;
  if (sound_processing_data::fast_source != 0) {
    al_call(alGetSourcei, sound_processing_data::fast_source, AL_SAMPLE_OFFSET, &samples_offset);
  }

  const size_t samples_count = basic_sound_data::res->sound->frames_count();
  const size_t processed_samples = sound_processing_data::processed_frames % samples_count + samples_offset;
  return double(processed_samples) / double(samples_count);
}

bool background_source::set_stat(const double place) {
  if (!is_playing()) return false;

  // тут надо дать понять что меняем место, как?
  // да просто поди выставим время в 0 и при обработке будет понятно че делать

  sound_processing_data::loaded_frames = place * currently_playing()->sound->frames_count();
  sound_processing_data::time = 0;
  return true;
}


void menu_source::set(float speed, float volume, float rnd_pitch, bool is_loop) {
  basic_sound_data::volume = volume;
  advanced_sound_data::speed = speed;
  advanced_sound_data::rnd_pitch = rnd_pitch;
  advanced_sound_data::is_loop = is_loop;

  if (sound_processing_data::fast_source == 0) return;
  const float final_vol = *basic_sound_data::type_volume * volume;
  al_call(alSourcef, sound_processing_data::fast_source, AL_GAIN, final_vol);

  // разобраться бы вот в этой штуке
  // AL_CONE_INNER_ANGLE
}

void menu_source::set_relative(const bool relative) {
  advanced_sound_data::is_rel = relative;
  if (sound_processing_data::fast_source != 0) {
    al_call(alSourcei, sound_processing_data::fast_source, AL_SOURCE_RELATIVE, int(relative));
  }
}

void special_source::set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) {
  full_sound_data::pos = pos;
  full_sound_data::dir = dir;
  full_sound_data::vel = vel;

  if (sound_processing_data::fast_source == 0) return;
  al_call(alSourcefv, sound_processing_data::fast_source, AL_POSITION, (float*)&pos);
  //al_call(alSourcefv, sound_processing_data::fast_source, AL_DIRECTION, (float*)&dir);
  al_call(alSourcefv, sound_processing_data::fast_source, AL_VELOCITY, (float*)&vel);
}
}
}