#pragma once

#include <cstdint>

namespace lsm9ds1
{
  namespace gyro
  {
    constexpr uint8_t ADDRESS = 0x6A;
    constexpr uint8_t DEVICE_ID = 0x68;

    namespace reg
    {

      constexpr uint8_t ACT_THS = 0x04;
      constexpr uint8_t ACT_DUR = 0x05;
      constexpr uint8_t INT_GEN_CFG_XL = 0x06;
      constexpr uint8_t INT_GEN_THS_X_XL = 0x07;
      constexpr uint8_t INT_GEN_THS_Y_XL = 0x08;
      constexpr uint8_t INT_GEN_THS_Z_XL = 0x09;
      constexpr uint8_t INT_GEN_DUR_XL = 0x0A;
      constexpr uint8_t REFERENCE_G = 0x0B;
      constexpr uint8_t INT1_CTRL = 0x0C;
      constexpr uint8_t INT2_CTRL = 0x0D;
      constexpr uint8_t WHO_AM_I = 0x0F;
      constexpr uint8_t CTRL_REG1_G = 0x10;
      constexpr uint8_t CTRL_REG2_G = 0x11;
      constexpr uint8_t CTRL_REG3_G = 0x12;
      constexpr uint8_t ORIENT_CFG_G = 0x13;
      constexpr uint8_t INT_GEN_SRC_G = 0x14;
      constexpr uint8_t OUT_TEMP_L = 0x15;
      constexpr uint8_t OUT_TEMP_H = 0x16;
      constexpr uint8_t STATUS_REG_G = 0x17;
      constexpr uint8_t OUT_X_L_G = 0x18;
      constexpr uint8_t OUT_X_H_G = 0x19;
      constexpr uint8_t OUT_Y_L_G = 0x1A;
      constexpr uint8_t OUT_Y_H_G = 0x1B;
      constexpr uint8_t OUT_Z_L_G = 0x1C;
      constexpr uint8_t OUT_Z_H_G = 0x1D;
      constexpr uint8_t CTRL_REG4 = 0x1E;
      constexpr uint8_t CTRL_REG5_XL = 0x1F;
      constexpr uint8_t CTRL_REG6_XL = 0x20;
      constexpr uint8_t CTRL_REG7_XL = 0x21;
      constexpr uint8_t CTRL_REG8 = 0x22;
      constexpr uint8_t CTRL_REG9 = 0x23;
      constexpr uint8_t CTRL_REG10 = 0x24;
      constexpr uint8_t INT_GEN_SRC_XL = 0x26;
      constexpr uint8_t STATUS_REG_XL = 0x27;
      constexpr uint8_t OUT_X_L_XL = 0x28;
      constexpr uint8_t OUT_X_H_XL = 0x29;
      constexpr uint8_t OUT_Y_L_XL = 0x2A;
      constexpr uint8_t OUT_Y_H_XL = 0x2B;
      constexpr uint8_t OUT_Z_L_XL = 0x2C;
      constexpr uint8_t OUT_Z_H_XL = 0x2D;
      constexpr uint8_t FIFO_CTRL = 0x2E;
      constexpr uint8_t FIFO_SRC = 0x2F;
      constexpr uint8_t INT_GEN_CFG_G = 0x30;
      constexpr uint8_t INT_GEN_THS_XH_G = 0x31;
      constexpr uint8_t INT_GEN_THS_XL_G = 0x32;
      constexpr uint8_t INT_GEN_THS_YH_G = 0x33;
      constexpr uint8_t INT_GEN_THS_YL_G = 0x34;
      constexpr uint8_t INT_GEN_THS_ZH_G = 0x35;
      constexpr uint8_t INT_GEN_THS_ZL_G = 0x36;
      constexpr uint8_t INT_GEN_DUR_G = 0x37;
    }
  }

  namespace mag
  {
    constexpr uint8_t ADDRESS = 0x1C;
    constexpr uint8_t DEVICE_ID = 0x3D;

    namespace reg
    {

      constexpr uint8_t OFFSET_X_REG_L_M = 0x05;
      constexpr uint8_t OFFSET_X_REG_H_M = 0x06;
      constexpr uint8_t OFFSET_Y_REG_L_M = 0x07;
      constexpr uint8_t OFFSET_Y_REG_H_M = 0x08;
      constexpr uint8_t OFFSET_Z_REG_L_M = 0x09;
      constexpr uint8_t OFFSET_Z_REG_H_M = 0x0A;
      constexpr uint8_t WHO_AM_I_M = 0x0F;
      constexpr uint8_t CTRL_REG1_M = 0x20;
      constexpr uint8_t CTRL_REG2_M = 0x21;
      constexpr uint8_t CTRL_REG3_M = 0x22;
      constexpr uint8_t CTRL_REG4_M = 0x23;
      constexpr uint8_t CTRL_REG5_M = 0x24;
      constexpr uint8_t STATUS_REG_M = 0x27;
      constexpr uint8_t OUT_X_L_M = 0x28;
      constexpr uint8_t OUT_X_H_M = 0x29;
      constexpr uint8_t OUT_Y_L_M = 0x2A;
      constexpr uint8_t OUT_Y_H_M = 0x2B;
      constexpr uint8_t OUT_Z_L_M = 0x2C;
      constexpr uint8_t OUT_Z_H_M = 0x2D;
      constexpr uint8_t INT_CFG_M = 0x30;
      constexpr uint8_t INT_SRC_M = 0x31;
      constexpr uint8_t INT_THS_L_M = 0x32;
      constexpr uint8_t INT_THS_H_M = 0x33;
    }
  }
}
