#pragma once

#include <cstdint>

namespace hts221 {
  constexpr uint8_t ADDRESS = 0x5F;
  constexpr uint8_t DEVICE_ID = 0xBC;

  namespace reg {
    constexpr uint8_t WHO_AM_I = 0x0F;
    constexpr uint8_t AV_CONF = 0x10;
    constexpr uint8_t CTRL_REG1 = 0x20;
    constexpr uint8_t CTRL_REG2 = 0x21;
    constexpr uint8_t CTRL_REG3 = 0x22;
    constexpr uint8_t STATUS_REG = 0x27;
    constexpr uint8_t HUMIDITY_OUT_L = 0x28;
    constexpr uint8_t HUMIDITY_OUT_H = 0x29;
    constexpr uint8_t TEMP_OUT_L = 0x2A;
    constexpr uint8_t TEMP_OUT_H = 0x2B;
    constexpr uint8_t H0_rH_x2 = 0x30;
    constexpr uint8_t H1_rH_x2 = 0x31;
    constexpr uint8_t T0_degC_x8 = 0x32;
    constexpr uint8_t T1_degC_x8 = 0x33;
    constexpr uint8_t T1_T0_MSB = 0x35;
    constexpr uint8_t H0_T0_OUT_L = 0x36;
    constexpr uint8_t H0_T0_OUT_H = 0x37;
    constexpr uint8_t H1_T0_OUT_L = 0x3A;
    constexpr uint8_t H1_T0_OUT_H = 0x3B;
    constexpr uint8_t T0_OUT_L = 0x3C;
    constexpr uint8_t T0_OUT_H = 0x3D;
    constexpr uint8_t T1_OUT_L = 0x3E;
    constexpr uint8_t T1_OUT_H = 0x3F;
  } // namespace reg

  namespace sampling {
    constexpr uint8_t AVGT_2 = 0x00;
    constexpr uint8_t AVGT_4 = 0x01;
    constexpr uint8_t AVGT_8 = 0x03;
    constexpr uint8_t AVGT_16 = 0x04;
    constexpr uint8_t AVGT_32 = 0x05;
    constexpr uint8_t AVGT_64 = 0x06;
    constexpr uint8_t AVGT_128 = 0x07;
    constexpr uint8_t AVGT_256 = 0x08;

    constexpr uint8_t AVGH_4 = 0x00;
    constexpr uint8_t AVGH_8 = 0x01;
    constexpr uint8_t AVGH_16 = 0x02;
    constexpr uint8_t AVGH_32 = 0x03;
    constexpr uint8_t AVGH_64 = 0x04;
    constexpr uint8_t AVGH_128 = 0x05;
    constexpr uint8_t AVGH_256 = 0x06;
    constexpr uint8_t AVGH_512 = 0x07;
  } // namespace sampling
} // namespace hts221
