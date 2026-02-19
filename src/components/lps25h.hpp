#pragma once

#include <cstdint>

namespace lps25h
{
  constexpr uint8_t ADDRESS = 0x5C;
  constexpr uint8_t DEVICE_ID = 0xBD;

  namespace reg
  {
    constexpr uint8_t WHO_AM_I = 0x0F;
  }
}
