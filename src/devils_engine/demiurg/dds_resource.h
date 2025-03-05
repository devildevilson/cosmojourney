#ifndef DEVILS_ENGINE_DEMIURG_DDS_RESOURCE_H
#define DEVILS_ENGINE_DEMIURG_DDS_RESOURCE_H

#include <cstdint>
#include <cstddef> 
#include "resource_base.h"

// так я тут между делом подумал что 
// нужно сделать что то вроде вью для ресурса
// ресурс вью
// где мы будем хранить небольшое количество данных о ресурсе
// и например именно это вью использовать в качестве передачи в другие места
// с другой стороны... зачем? я могу получить пачку указателей из основной системы
// в любом виде и оттуда все что мне нужно брать
// единственное это когда мы будем делать require в луа
// нужно динамически понять какой тип у нас приходит и соответственно его отдать
// наверное просто сделаем 

namespace devils_engine {
namespace demiurg {
// к картинкам будет базовый класс
// в котором будут лежать полезные вещи всякие

class dds_resource_loader : public resource_interface {
public:
  dds_resource_loader() noexcept;

  // а хотя в каком виде то подгружать? парсить ничего не нужно
  // значит нужно сразу грузить в рендер, которого сейчас нет =(
  virtual void load_mem(std::vector<uint8_t> mem);
protected:

};
}
}

#endif