#pragma once

#include <cstdint>

#include <spdlog/spdlog.h>

#include "i2c.hpp"
#include "components/hts221.hpp"
#include "components/lps25h.hpp"
#include "components/lsm9ds1.hpp"

class SenseHat
{
public:
  SenseHat()
      : _bus("/dev/i2c-1"),
        _humiditySensor(this->_bus, "Humidity Sensor", hts221::ADDRESS),
        _pressureSensor(this->_bus, "Pressure Sensor", lps25h::ADDRESS),
        _gyroAccelSensor(this->_bus, "Gyroscope/Accelerometer Sensor", lsm9ds1::gyro::ADDRESS),
        _magnetometerSensor(this->_bus, "Magnetometer Sensor", lsm9ds1::mag::ADDRESS)
  {
    this->_humiditySensor.writeByte(hts221::reg::CTRL_REG1, 0x85);

    spdlog::debug("SenseHat initialized successfully");
  }

  void testHardware() const
  {
    spdlog::info("Testing SenseHat hardware...");

    bool result = true;

    result &= this->checkHardwareId(this->_humiditySensor, hts221::reg::WHO_AM_I, hts221::DEVICE_ID);
    result &= this->checkHardwareId(this->_pressureSensor, lps25h::reg::WHO_AM_I, lps25h::DEVICE_ID);
    result &= this->checkHardwareId(this->_gyroAccelSensor, lsm9ds1::gyro::reg::WHO_AM_I, lsm9ds1::gyro::DEVICE_ID);
    result &= this->checkHardwareId(this->_magnetometerSensor, lsm9ds1::mag::reg::WHO_AM_I_M, lsm9ds1::mag::DEVICE_ID);

    if (result)
    {
      spdlog::info("✅ All SenseHat hardware components verified successfully");
    }
    else
    {
      spdlog::error("❌ One or more SenseHat hardware components failed verification. Please review the error messages above for details.");
    }
  }

  float readTemperature() const
  {
    return this->readTemperature(false);
  }

  float readTemperature(bool asFahrenheit) const
  {

    const uint8_t tempCalPoint0Lsb = this->_humiditySensor.readByte(hts221::reg::T0_degC_x8);
    const uint8_t tempCalPoint1Lsb = this->_humiditySensor.readByte(hts221::reg::T1_degC_x8);
    const uint8_t tempCalPointMsb = this->_humiditySensor.readByte(hts221::reg::T1_T0_MSB);

    const uint16_t tempCalPoint0Msb = (tempCalPointMsb & 0x03) << 8;
    const uint16_t tempCalPoint1Msb = ((tempCalPointMsb & 0x0C) >> 2) << 8;

    const uint16_t tempCalPoint0_x8 = static_cast<uint16_t>(tempCalPoint0Lsb) | tempCalPoint0Msb;
    const uint16_t tempCalPoint1_x8 = static_cast<uint16_t>(tempCalPoint1Lsb) | tempCalPoint1Msb;

    const float tempCalPoint0 = static_cast<float>(tempCalPoint0_x8) / 8.0f;
    const float tempCalPoint1 = static_cast<float>(tempCalPoint1_x8) / 8.0f;

    const int16_t temp0Raw = this->_humiditySensor.readShort(hts221::reg::T0_OUT_L, hts221::reg::T0_OUT_H);
    const int16_t temp1Raw = this->_humiditySensor.readShort(hts221::reg::T1_OUT_L, hts221::reg::T1_OUT_H);
    const int16_t tempRaw = this->_humiditySensor.readShort(hts221::reg::TEMP_OUT_L, hts221::reg::TEMP_OUT_H);

    if (temp0Raw == temp1Raw)
    {
      spdlog::error("Invalid temperature calibration data: T0_OUT and T1_OUT raw values are identical ({}). Cannot perform interpolation.", temp0Raw);
      return 0.0f;
    }

    const float temperature = tempCalPoint0 + (static_cast<float>(tempRaw - temp0Raw) * (tempCalPoint1 - tempCalPoint0) / static_cast<float>(temp1Raw - temp0Raw));

    if (!asFahrenheit)
    {
      return temperature;
    }
    else
    {
      return (temperature * (9.0f / 5.0f)) + 32.0f;
    }
  }

private:
  i2c::Bus _bus;
  i2c::Device _humiditySensor;
  i2c::Device _pressureSensor;
  i2c::Device _gyroAccelSensor;
  i2c::Device _magnetometerSensor;

  struct HumiditySensorCalibration
  {
  };

  bool checkHardwareId(const i2c::Device &device, uint8_t whoAmIReg, uint8_t expectedId) const
  {
    const uint8_t actualId = device.readByte(whoAmIReg);

    if (expectedId != actualId)
    {
      spdlog::error("❌ {} hardware ID mismatch: expected 0x{:02X}, got 0x{:02X}", device.name(), expectedId, actualId);
      return false;
    }

    return true;
  }
};
