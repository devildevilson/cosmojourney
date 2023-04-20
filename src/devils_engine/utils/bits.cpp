/* Modified version of https://github.com/wroniasty/bits */
#include <cstdint>
#include <cstring>
#include "bits.h"

namespace devils_engine {
  namespace utils {
    std::string hexdump(const uint8_t* buffer, const size_t length, const size_t line_size) {
      std::string dump;
      std::string _hex;
      std::string _ascii;
      _hex.reserve(line_size*2);
      _ascii.reserve(line_size*2);
      for (size_t i = 0; i < length; i += line_size) {
        for (size_t j = i; j < i+line_size; j++) {
          const size_t mask = 0xff;
          const uint8_t first_half = buffer[j] & mask;
          const uint8_t second_half = buffer[j] >> 4;
          _hex.push_back(hexstr(second_half));
          _hex.push_back(hexstr(first_half));
          if (j%4 == 3) _hex.push_back(' ');
          _ascii.push_back((buffer[j] >= ' ' && buffer[j] <= 'z') ? buffer[j] : '.');
        }

        dump = dump + _hex + "   " + _ascii + "\n";
        _hex.clear();
        _ascii.clear();
      }
      return dump;
    }
    
    std::string binstr(const uint8_t v) {
      std::string s; 
      uint8_t m = 128;

      while (m > 0) {
        s += (v & m) ? "1" : "0";
        m >>= 1;
      }

      return s;
    } 

    std::string binstr(const std::span<uint8_t> &arr) {
      std::string s;
      size_t current = 0;
      for (size_t i = 0; i < arr.size(); ++i) {
        s += binstr(arr[i]); 
      }
      return s;
    }

    // uint8_t setbits(const uint8_t c, const size_t offset, const size_t numbits, const uint8_t v) {
    //   uint8_t stamp = (v    << (uint8_significant_bits_count-numbits)), 
    //           mask  = (0xff << (uint8_significant_bits_count-numbits));
    //   stamp >>= offset; 
    //   mask = ~(mask >>= offset);
    //   return (c & mask) | stamp; 
    // }
  }
}
