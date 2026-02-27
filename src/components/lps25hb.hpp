#pragma once

#include <cstdint>

namespace lps25hb {
  constexpr uint8_t ADDRESS = 0x5C;
  constexpr uint8_t DEVICE_ID = 0xBD;

  namespace reg {
    constexpr uint8_t WHO_AM_I = 0x0F;
  }
} // namespace lps25hb
