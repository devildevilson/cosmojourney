#ifndef DEVILS_ENGINE_SOUND_VIRTUAL_SOURCE_H
#define DEVILS_ENGINE_SOUND_VIRTUAL_SOURCE_H

#include <cstddef>
#include <cstdint>
#include "glm/vec3.hpp"
#include "resource.h"

namespace devils_engine {
namespace sound {

// может быть переназвать неймспейс?

// нужно ли приставить к виртуальному сорсу собственные буферы?
// типа отрабатывать обычный сценарий со звуком при его появлении
// скорее нет чем да, лучше немного потрудиться и написать старт по времени
// желательно добиться того чтобы звук не прерывался никак
// + возможно нужно интерполировать положение звука?
// если 30фпс даже берем то вряд ли
class virtual_source {
public:
  struct settings {
    uint32_t type; // тип наверное укажем заранее, так уйдет и is_mono
    float speed;
    float volume;
    float rnd_pitch;
    //float rnd_rate; // не будет
    bool is_loop;
    bool is_mono;
    bool is_needed; // если все источники сейчас заняты, то дропаем звук - подойдет для мелких звуков
    uint32_t force_source; // не уверен что это теперь нужно вообще
    // координаты
    //glm::vec3 pos;
    //glm::vec3 dir;
    //glm::vec3 vel;

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


  virtual ~virtual_source() noexcept = default;

  // тут нужно задать все настроечки

  // стартует звук, потом обнуляет указатель
  // либо его нужно будет обнулить если это залупленный звук
  virtual void set_resource(const resource* res) = 0;
  virtual bool is_playing() const = 0;
  virtual resource* currently_playing() const = 0;

  //virtual void set(const settings &s) = 0;
  // наверное чаще нам придется задавать скорость нежели чем громкость звука
  // громкость определяется типом звука и настраивается через настроечки
  // мы дополнительно можем ее уменьшить
  virtual void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) = 0;
  virtual void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) = 0;
  virtual void set_relative(const bool relative) = 0;
};

struct basic_sound_data {
  const resource* res;
  float* type_volume;
  float volume;
  inline basic_sound_data() noexcept : res(nullptr), type_volume(nullptr), volume(1.0f) {}
};

struct advanced_sound_data : public basic_sound_data {
  float speed; 
  float rnd_pitch; 
  bool is_loop;
  bool is_mono;
  inline advanced_sound_data() noexcept : speed(1.0f), rnd_pitch(0.0f), is_loop(false), is_mono(true) {}
};

// relative?
struct full_sound_data : public advanced_sound_data {
  glm::vec3 pos;
  glm::vec3 dir;
  glm::vec3 vel;
  inline full_sound_data() noexcept : pos(0.0f, 0.0f, 0.0f), dir(0.0f, 0.0f, 0.0f), vel(0.0f, 0.0f, 0.0f) {}
};

// будет один стерео источник
class background_source : public basic_sound_data, public virtual_source {
public:
  
  void set_resource(const resource* res) override;
  bool is_playing() const override;
  resource* currently_playing() const override;
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
  void set_relative(const bool relative) override;
};

// несколкьо меню звуокв наверное должны иметь возможность играть одновременно
// они все скорее всего будут мелкими, наверное для меню нужно будет создать типа 3-5 сорвсов
// и менять их просто друг за другом в очереди
class menu_source : public advanced_sound_data, public virtual_source {
public:
  void set_resource(const resource* res) override;
  bool is_playing() const override;
  resource* currently_playing() const override;
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
  void set_relative(const bool relative) override;
};

// их может быть штук 10, наверное для них должны быть эксклюзивные сорсы
// это что то важное в геймплее, например высказывание персонажа или важный игровой скилл
class special_source : public full_sound_data, public virtual_source {
public:
  void set_resource(const resource* res) override;
  bool is_playing() const override;
  resource* currently_playing() const override;
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
  void set_relative(const bool relative) override;
};

// основные игровые звуки, будет по количеству объектов даже больше
// в большинстве случаев будут простаивать либо по причине отсутсвия ресурса
// либо по причине сильной удаленности звука
// ищем валидные сорсы которые относительно недалеко от слушателя
// и закидываем туда реальные сорсы, после того как звук прекратился
// выкидываем сорс, у нас теоретически может быть ситуация когда сорсов 
// не хватит на все звуки в сцене и тогда я пока не понимаю что делать
// эти звуки тоже должны быть относительно мелкими
class game_source : public full_sound_data, public virtual_source {
public:
  void set_resource(const resource* res) override;
  bool is_playing() const override;
  resource* currently_playing() const override;
  void set(float speed, float volume = 1.0f, float rnd_pitch = 0.0f, bool is_loop = false) override;
  void set_transform(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &vel) override;
  void set_relative(const bool relative) override;
};

// имеет смысл еще поверх этого дела написать класс который положит в очередь 
// звуки и предоставит какую то обратную связь, например
// нам имеет смысл проверить когда закачивается тот или иной специальный звук
// его приостановить и начать какой нибудь другой
}
}

#endif