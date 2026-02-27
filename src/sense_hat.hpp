#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <string_view>

#include "components/hts221.hpp"
#include "components/lps25hb.hpp"
#include "components/lsm9ds1.hpp"
#include "i2c.hpp"

namespace {
  struct DefaultLogger {
    static void trace(std::string_view msg) {}
    static void debug(std::string_view msg) {}
    static void info(std::string_view msg) {}
    static void warn(std::string_view msg) {}
    static void error(std::string_view msg) {}
    static void critical(std::string_view msg) {}
  };
} // namespace

template <typename Logger = DefaultLogger>
class SenseHat {
public:
  explicit SenseHat(Logger logger = Logger{}) :
      _logger(logger),
      _bus("/dev/i2c-1"),
      _humiditySensor(this->_bus, "Humidity Sensor", hts221::ADDRESS),
      _pressureSensor(this->_bus, "Pressure Sensor", lps25hb::ADDRESS),
      _magSensor(this->_bus, "Magnetometer Sensor", lsm9ds1::mag::ADDRESS),
      _gyroAccelSensor(this->_bus, "Gyroscope/Accelerometer Sensor", lsm9ds1::gyro::ADDRESS) {
    // TODO: Replace 0x85 with its constituant flags
    this->_humiditySensor.writeByte(hts221::reg::CTRL_REG1, 0x85);
  }

  ~SenseHat() = default;

  struct SensorOffsets {
    double SimpleCompensationTemperatureOffset{0.0};
    double LinearCompensationTemperatureScale{0.0};
    double LinearCompensationTemperatureOffset{0.0};
    double CpuCompensationCpuCoefficient{0.0};
  };

  void testHardware() const {
    this->_logger.info("Testing SenseHat hardware...");

    bool res = true;

    res &= SenseHat::checkHardwareId(this->_humiditySensor, hts221::reg::WHO_AM_I, hts221::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_pressureSensor, lps25hb::reg::WHO_AM_I, lps25hb::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_magSensor, lsm9ds1::mag::reg::WHO_AM_I_M, lsm9ds1::mag::DEVICE_ID);
    res &= SenseHat::checkHardwareId(this->_gyroAccelSensor, lsm9ds1::gyro::reg::WHO_AM_I, lsm9ds1::gyro::DEVICE_ID);

    if (res) {
      this->_logger.info("✅ All SenseHat hardware components verified successfully");
    } else {
      this->_logger.error("❌ One or more SenseHat hardware components failed "
                          "verification. Please review the error "
                          "messages above for details.");
    }
  }

  double readTemperature() const { return this->readTemperature(false); }

  double readTemperature(bool asFahrenheit) const {
    const uint8_t tempCalPoint0Lsb = this->_humiditySensor.readByte(hts221::reg::T0_degC_x8);
    const uint8_t tempCalPoint1Lsb = this->_humiditySensor.readByte(hts221::reg::T1_degC_x8);
    const uint8_t tempCalPointMsb = this->_humiditySensor.readByte(hts221::reg::T1_T0_MSB);

    const uint16_t tempCalPoint0Msb = (tempCalPointMsb & 0x03) << 8;
    const uint16_t tempCalPoint1Msb = (tempCalPointMsb & 0x0C) << 6;

    const uint16_t tempCalPoint0_x8 = static_cast<uint16_t>(tempCalPoint0Lsb) | tempCalPoint0Msb;
    const uint16_t tempCalPoint1_x8 = static_cast<uint16_t>(tempCalPoint1Lsb) | tempCalPoint1Msb;

    const double tempCalPoint0 = static_cast<double>(tempCalPoint0_x8) / 8.0;
    const double tempCalPoint1 = static_cast<double>(tempCalPoint1_x8) / 8.0;

    const int16_t temp0Raw = this->_humiditySensor.readShort(hts221::reg::T0_OUT_L, hts221::reg::T0_OUT_H);
    const int16_t temp1Raw = this->_humiditySensor.readShort(hts221::reg::T1_OUT_L, hts221::reg::T1_OUT_H);
    const int16_t tempRaw = this->_humiditySensor.readShort(hts221::reg::TEMP_OUT_L, hts221::reg::TEMP_OUT_H);

    if (temp0Raw == temp1Raw) {
      this->_logger.error(std::format("Invalid temperature calibration data: T0_OUT and T1_OUT "
                                      "raw values are identical ({}). Cannot "
                                      "perform interpolation.",
                                      temp0Raw));
      return 0.0;
    }

    const double temperature = tempCalPoint0 +
                               ((tempRaw - temp0Raw) * (tempCalPoint1 - tempCalPoint0) / (temp1Raw - temp0Raw));

    if (!asFahrenheit) {
      return temperature;
    }

    return (temperature * (9.0 / 5.0)) + 32.0;
  }

  double readHumidity() const {
    const uint8_t humidityCalPoint0_x2 = this->_humiditySensor.readByte(hts221::reg::H0_rH_x2);
    const uint8_t humidityCalPoint1_x2 = this->_humiditySensor.readByte(hts221::reg::H1_rH_x2);

    const double humidityCalPoint0 = humidityCalPoint0_x2 / 2.0;
    const double humidityCalPoint1 = humidityCalPoint1_x2 / 2.0;

    const int16_t humidity0Raw = this->_humiditySensor.readShort(hts221::reg::H0_T0_OUT_L, hts221::reg::H0_T0_OUT_H);
    const int16_t humidity1Raw = this->_humiditySensor.readShort(hts221::reg::H1_T0_OUT_L, hts221::reg::H1_T0_OUT_H);
    const int16_t humidityRaw = this->_humiditySensor.readShort(hts221::reg::HUMIDITY_OUT_L,
                                                                hts221::reg::HUMIDITY_OUT_H);

    if (humidity0Raw == humidity1Raw) {
      this->_logger.error(std::format("Invalid humidity calibration data: H1_T0_OUT and H0_T0_OUT "
                                      "raw values are identical ({}). Cannot "
                                      "perform interpolation.",
                                      humidity0Raw));

      return 0.0;
    }

    const double humidity = humidityCalPoint0 +
                            ((humidityRaw - humidity0Raw) * (humidityCalPoint1 - humidityCalPoint0) /
                             (humidity1Raw - humidity0Raw));

    return std::clamp(humidity, 0.0, 100.0);
  }

private:
  Logger _logger;
  i2c::Bus _bus;
  i2c::Device _humiditySensor;
  i2c::Device _pressureSensor;
  i2c::Device _magSensor;
  i2c::Device _gyroAccelSensor;
  SensorOffsets _offsets{};

  struct HumiditySensorCalibration {};

  bool checkHardwareId(const i2c::Device &device, uint8_t whoAmIReg, uint8_t expectedId) const {
    const uint8_t actualId = device.readByte(whoAmIReg);

    if (expectedId != actualId) {
      this->_logger.error(std::format("❌ {} hardware ID mismatch: expected 0x{:02X}, got 0x{:02X}",
                                      device.name(),
                                      expectedId,
                                      actualId));
      return false;
    }

    this->_logger.info(std::format("✅ {} hardware ID verified: 0x{:02X}", device.name(), actualId));

    return true;
  }
};
