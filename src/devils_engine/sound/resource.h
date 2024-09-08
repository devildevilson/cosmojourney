#ifndef DEVILS_ENGINE_SOUND_RESOURCE_H
#define DEVILS_ENGINE_SOUND_RESOURCE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>
#include "decoder.h"

namespace devils_engine {
namespace sound {

#define SOUND_SYSTEM_EXTENSION_LIST \
  X(mp3)  \
  X(flac) \
  X(wav)  \
  X(ogg)  \
  X(pcm)  \

struct resource {
  enum class type {
#define X(name) name,
    SOUND_SYSTEM_EXTENSION_LIST
#undef X

    undefined
  };

  static std::string_view type_to_string(const size_t index);
        
  // теперь не нужно
  std::string id;
  enum type type;
  std::unique_ptr<sound::decoder> sound;
  std::vector<char> buffer;

  resource();
  // некоторые ресурсы нужно перевести сразу в pcm формат
  resource(std::string id, enum type type, std::vector<char> buffer);
  ~resource() noexcept;

  double duration() const; // s
};
}
}

#endif