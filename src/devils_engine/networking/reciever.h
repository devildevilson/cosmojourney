#ifndef DEVILS_ENGINE_NETWORKING_RECIEVER_H
#define DEVILS_ENGINE_NETWORKING_RECIEVER_H

#include <cstddef>
#include <cstdint>
#include <tuple>

namespace devils_engine {
namespace networking {
  class reciever {
  public:

    template<typename T, size_t id>
    void register_type() {
      // сопоставляем размер и id
      // нет, тип может быть с данными произвольного размера
      // при записи в пакет надо бы указать размер в байтах (а можно наверное считерить и указать размер в 4х байтах)
      // 
    }

    // тут лучше принимать байты в заранее созданном буфере
    size_t next(char* mem, const size_t max_mem) const {
      // парсим следующие в списке данные ... какой то кольцевой буфер тут будет наверное 
    }
  };

  // когда мы отправляем? я думал что отправка/прием будут автоматом как то делаться
  // но при этом желательно делать в одно и тоже время
  // + нужно записывать текущий игровой тик
  // как будто бы сендер должен быть синхронизирован с основным симулятором
  // а ресивер как придется
  // пакет надо бы держать в пределах 1500 байт
  // сверху этого нужно написать структуру по снапшотам
  // где мы будем подтверждать те или иные вещи и уметь откатывать и заново перевычислять мир
  // например в кваке 250% пакеты с перемещением это пакеты которые игра легко откидывает 
  // а вот пакеты с убийством и выстрелами как будто игра ждет пока придут 
  // в случае сложной физики походу все пакеты нужно ждать, и чисто в них передавать изменения в мире
  // да такое ощущение что пока я не делаю компетитив шутер мне можно просто использовать UDP
  // с какой то версией надежности, в остальном же вопрос скорее в том что положить в пакет
  // советуют отправлять по 256 байт 30 раз в секунду (это 61 кбит/с), что можно положить в 256 байт?
  // 256 байт это 64 4 байтных числа тут можно много чего положить, думаю что нужно сначала 
  // написать какую то первую версию сендера и ресивера, а потом глянуть че как 
  // сможет ли алпака справится с упаковкой?
  class sender {
  public:

    template <typename T>
    void add(T data) {
      // здесь поди будет какой то массив в который мы сначала запишем тип (?)
      // затем записанный размер укажем в первых двух байтах? а может для алпаки размер неважен
      // после чего для всех данных будет указан тип и после него уже полезные данные
    }

    void commit();
  };
}
}

#endif