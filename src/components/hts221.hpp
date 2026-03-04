#pragma once

#include <algorithm>
#include <cstdint>
#include <expected>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "../i2c.hpp"

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

  namespace flags {
    constexpr uint8_t POWER_MODE = 0x80;
    constexpr uint8_t UPDATE_MODE = 0x04;
    constexpr uint8_t DATA_RATE_ONE_SHOT = 0x00;
    constexpr uint8_t DATA_RATE_1_HZ = 0x01;
    constexpr uint8_t DATA_RATE_7_HZ = 0x02;
    constexpr uint8_t DATA_RATE_12_HZ = 0x03;
    constexpr uint8_t AUTO_INC = 0x80;
  } // namespace flags
} // namespace hts221

class Hts221 : public i2c::Device {
public:
  explicit Hts221(i2c::Bus &bus) :
      i2c::Device(bus, "Humidity Sensor", hts221::DEVICE_ID) {}

  ~Hts221() = default;

  void initialize() const {
    this->writeByte(hts221::reg::CTRL_REG1,
                    hts221::flags::POWER_MODE | hts221::flags::UPDATE_MODE | hts221::flags::DATA_RATE_1_HZ);
  }

  [[nodiscard]] double getTemperature() const { return this->getTemperature(false); };

  [[nodiscard]] double getTemperature(bool asFahrenheit) const {
    const uint8_t tempCalPoint0Lsb = this->readByte(hts221::reg::T0_degC_x8);
    const uint8_t tempCalPoint1Lsb = this->readByte(hts221::reg::T1_degC_x8);
    const uint8_t tempCalPointMsb = this->readByte(hts221::reg::T1_T0_MSB);

    const uint16_t tempCalPoint0Msb = (tempCalPointMsb & 0x03) << 8;
    const uint16_t tempCalPoint1Msb = (tempCalPointMsb & 0x0C) << 6;

    const uint16_t tempCalPoint0_x8 = static_cast<uint16_t>(tempCalPoint0Lsb) | tempCalPoint0Msb;
    const uint16_t tempCalPoint1_x8 = static_cast<uint16_t>(tempCalPoint1Lsb) | tempCalPoint1Msb;

    const double tempCalPoint0 = static_cast<double>(tempCalPoint0_x8) / 8.0;
    const double tempCalPoint1 = static_cast<double>(tempCalPoint1_x8) / 8.0;

    const int16_t temp0Raw = this->readShort(hts221::reg::T0_OUT_L);
    const int16_t temp1Raw = this->readShort(hts221::reg::T1_OUT_L);
    const int16_t tempRaw = this->readShort(hts221::reg::TEMP_OUT_L);

    if (temp0Raw == temp1Raw) {
      return 0;
    };

    const double temperature = tempCalPoint0 +
                               ((tempRaw - temp0Raw) * (tempCalPoint1 - tempCalPoint0) / (temp1Raw - temp0Raw));

    if (!asFahrenheit) {
      return temperature;
    }

    return (temperature * (9.0 / 5.0)) + 32.0;
  }

  [[nodiscard]] std::expected<double, std::error_code> getHumidity() const {
    const auto humidityCalPoint0_x2 = TRY(this->readByte(hts221::reg::H0_rH_x2));
    const auto humidityCalPoint1_x2 = TRY(this->readByte(hts221::reg::H1_rH_x2));

    const auto humidity0Raw = TRY(this->readShort(hts221::reg::H0_T0_OUT_L));
    const auto humidity1Raw = TRY(this->readShort(hts221::reg::H1_T0_OUT_L));
    const auto humidityRaw = TRY(this->readShort(hts221::reg::HUMIDITY_OUT_L));

    if (humidity0Raw == humidity1Raw) {
      return 0;
    }

    const double humidityCalPoint0 = humidityCalPoint0_x2 / 2.0;
    const double humidityCalPoint1 = humidityCalPoint1_x2 / 2.0;

    const double humidity = humidityCalPoint0 +
                            ((humidityRaw - humidity0Raw) * (humidityCalPoint1 - humidityCalPoint0) /
                             (humidity1Raw - humidity0Raw));

    return std::clamp(humidity, 0.0, 100.0);
  }

  [[nodiscard]] std::vector<std::pair<std::string, bool>> test() override {
    std::vector<std::pair<std::string, bool>> results{};

    results.emplace_back("Device ID Correct", hts221::DEVICE_ID == this->readByte(hts221::reg::WHO_AM_I));

    return results;
  }

private:
  // i2c::Device sensor;
};
