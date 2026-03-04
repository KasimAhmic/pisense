#pragma once

#include <cstdint>
#include <string>

#include "components/hts221.hpp"
#include "components/lps25hb.hpp"
#include "components/lsm9ds1.hpp"
#include "i2c.hpp"

class SenseHat {
public:
  explicit SenseHat() :
      _bus("/dev/i2c-1"),
      _humiditySensor(this->_bus),
      _pressureSensor(this->_bus, "Pressure Sensor", lps25hb::ADDRESS),
      _magSensor(this->_bus, "Magnetometer Sensor", lsm9ds1::mag::ADDRESS),
      _gyroAccelSensor(this->_bus, "Gyroscope/Accelerometer Sensor", lsm9ds1::gyro::ADDRESS) {
    this->_humiditySensor.initialize();
  }

  explicit SenseHat(const std::string &bus) :
      _bus(bus),
      _humiditySensor(this->_bus),
      _pressureSensor(this->_bus, "Pressure Sensor", lps25hb::ADDRESS),
      _magSensor(this->_bus, "Magnetometer Sensor", lsm9ds1::mag::ADDRESS),
      _gyroAccelSensor(this->_bus, "Gyroscope/Accelerometer Sensor", lsm9ds1::gyro::ADDRESS) {
    this->_humiditySensor.initialize();
  }

  ~SenseHat() = default;

  struct SensorOffsets {
    double SimpleCompensationTemperatureOffset{0.0};
    double LinearCompensationTemperatureScale{0.0};
    double LinearCompensationTemperatureOffset{0.0};
    double CpuCompensationCpuCoefficient{0.0};
  };

  void testHardware() const {
    // this->_logger.info("Testing SenseHat hardware...");

    bool res = true;
    this->_humiditySensor.readByte(1);

    res &= SenseHat::checkHardwareId(this->_humiditySensor, hts221::reg::WHO_AM_I, hts221::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_pressureSensor, lps25hb::reg::WHO_AM_I, lps25hb::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_magSensor, lsm9ds1::mag::reg::WHO_AM_I_M, lsm9ds1::mag::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_gyroAccelSensor, lsm9ds1::gyro::reg::WHO_AM_I, lsm9ds1::gyro::DEVICE_ID);

    if (res) {
      // this->_logger.info("✅ All SenseHat hardware components verified successfully");
    } else {
      // this->_logger.error("❌ One or more SenseHat hardware components failed "
      //                     "verification. Please review the error "
      //                     "messages above for details.");
    }
  }

  double getTemperature() const { return this->getTemperature(false); }
  double getTemperature(bool asFahrenheit) const { return this->_humiditySensor.getTemperature(asFahrenheit); }
  double getHumidity() const { return this->_humiditySensor.getHumidity(); }

private:
  i2c::Bus _bus;
  Hts221 _humiditySensor;
  i2c::Device _pressureSensor;
  i2c::Device _magSensor;
  i2c::Device _gyroAccelSensor;
  SensorOffsets _offsets{};

  struct HumiditySensorCalibration {};

  bool checkHardwareId(const i2c::Device &device, uint8_t whoAmIReg, uint8_t expectedId) const {
    const uint8_t actualId = device.readByte(whoAmIReg);

    if (expectedId != actualId) {
      // this->_logger.error(std::format("❌ {} hardware ID mismatch: expected 0x{:02X}, got 0x{:02X}",
      //                                 device.name(),
      //                                 expectedId,
      //                                 actualId));
      return false;
    }

    // this->_logger.info(std::format("✅ {} hardware ID verified: 0x{:02X}", device.name(), actualId));

    return true;
  }
};
