#ifndef DEVILS_ENGINE_SOUND_BASIC_SOURCES_H
#define DEVILS_ENGINE_SOUND_BASIC_SOURCES_H

#include <cstddef>
#include <cstdint>
#include "virtual_source.h"

namespace devils_engine {
namespace sound {
// данные по обработке звука тут?
struct sound_processing_data {
  size_t time;
  size_t loaded_frames;
  size_t processed_frames;
  uint32_t fast_source;
  inline sound_processing_data() noexcept : time(0), loaded_frames(0), processed_frames(0), fast_source(0) {}
};

struct basic_sound_data {
  const resource* res;
  const resource* prev_res;
  float* type_volume;
  float volume;
  inline basic_sound_data() noexcept : res(nullptr), prev_res(nullptr), type_volume(nullptr), volume(1.0f) {}
};

struct advanced_sound_data {
  float speed; 
  float rnd_pitch; 
  bool is_loop;
  bool is_mono;
  bool is_rel;
  inline advanced_sound_data() noexcept : speed(1.0f), rnd_pitch(0.0f), is_loop(false), is_mono(true), is_rel(true) {}
};

// relative?
struct full_sound_data {
  glm::vec3 pos;
  glm::vec3 dir;
  glm::vec3 vel;
  inline full_sound_data() noexcept : pos(0.0f, 0.0f, 0.0f), dir(0.0f, 0.0f, 0.0f), vel(0.0f, 0.0f, 0.0f) {}
};

struct source {
  uint32_t source;
  uint32_t buffers[2];
};

enum class processing_state { initial, waiting_source, processing, paused, finished };

// мы должны как то понять что отсюда нужно забрать сорс? processing_state::finished
// как положить и как обратно добавить?
// сорс наверное нужно проинициализировать 
class source_processing {
public:
  virtual ~source_processing() noexcept = default;
  virtual processing_state state() const = 0;
  virtual float distance(const glm::vec3 &listener_pos) const = 0;
  virtual void update(const size_t time) = 0;
  virtual void setup_source(const struct source &source) = 0;
  virtual struct source release_source() = 0;
};

// будет один стерео источник
class background_source : public virtual_source, public sound_processing_data, public basic_sound_data {
public:
  void set_resource(const resource* res) override;
  bool is_valid() const override;
  const resource* currently_playing() const override;
  // здесь желательно устанавливать значения сразу в сорсе, а можем мы так делать?
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
  void set_relative(const bool relative) override;

  // если мы захотим делать паузу на источниках
  // то поди нужно разделить состояние когда у нас есть звук
  // и когда у нас просто пауза

  bool play() override;
  bool pause() override;
  bool is_playing() const override;
  bool is_paused() const override;

  double stat() const override;
  // после этого как то надо переинициализировать обработку наверное?
  bool set_stat(const double place) override;
};

// несколкьо меню звуокв наверное должны иметь возможность играть одновременно
// они все скорее всего будут мелкими, наверное для меню нужно будет создать типа 3-5 сорвсов
// и менять их просто друг за другом в очереди
class menu_source : public background_source, public advanced_sound_data {
public:
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_relative(const bool relative) override;
};

// их может быть штук 10, наверное для них должны быть эксклюзивные сорсы
// это что то важное в геймплее, например высказывание персонажа или важный игровой скилл
class special_source : public menu_source, public full_sound_data {
public:
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
};

// основные игровые звуки, будет по количеству объектов даже больше
// в большинстве случаев будут простаивать либо по причине отсутсвия ресурса
// либо по причине сильной удаленности звука
// ищем валидные сорсы которые относительно недалеко от слушателя
// и закидываем туда реальные сорсы, после того как звук прекратился
// выкидываем сорс, у нас теоретически может быть ситуация когда сорсов 
// не хватит на все звуки в сцене и тогда я пока не понимаю что делать
// эти звуки тоже должны быть относительно мелкими
class game_source : public special_source {
public:

};

// обработку в самом типе звука сделать? тут нужно сделать функцию передачи сорса
// функцию освобождения сорса и функцию обновления всего этого дела
}
}

#endif