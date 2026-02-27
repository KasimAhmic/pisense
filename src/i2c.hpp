#pragma once

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

#include <spdlog/spdlog.h>
#include <sys/ioctl.h>

namespace i2c {
  class Bus {
  public:
    explicit Bus(std::string bus) :
        _bus(std::move(bus)) {
      spdlog::debug("Opening I2C bus: {}", this->_bus);

      this->_fd = ::open(this->_bus.c_str(), O_RDWR);

      if (this->_fd < 0) {
        spdlog::error("Failed to open I2C bus {}: {}", this->_bus, strerror(errno));
        throw std::runtime_error("Failed to open I2C bus");
      }

      spdlog::debug("I2C bus {} opened successfully: fd={}", this->_bus, this->_fd);
    }

    ~Bus() noexcept {
      if (this->_fd >= 0) {
        spdlog::debug("Closing I2C bus {}: fd={}", this->_bus, this->_fd);

        if (::close(this->_fd) < 0) {
          spdlog::error("Failed to close I2C bus {}: fd={} | error={}", this->_bus, this->_fd, strerror(errno));
        } else {
          spdlog::debug("I2C bus {} closed successfully: fd={}", this->_bus, this->_fd);
        }
      }
    }

    Bus(const Bus &) = delete;
    Bus &operator=(const Bus &) = delete;

    Bus(Bus &&other) noexcept :
        _bus(std::move(other._bus)), _fd(other._fd), _activeAddr(other._activeAddr) {
      other._fd = -1;
      other._activeAddr = -1;
    }

    Bus &operator=(Bus &&other) noexcept {
      if (this == &other) {
        return *this;
      }

      if (this->_fd >= 0) {
        ::close(this->_fd);
      }

      this->_bus = std::move(other._bus);
      this->_fd = other._fd;
      this->_activeAddr = other._activeAddr;
      other._fd = -1;
      other._activeAddr = -1;

      return *this;
    }

    int fd() const noexcept { return this->_fd; }

    void setAddress(uint8_t addr) const {
      if (this->_activeAddr == static_cast<int>(addr)) {
        return;
      }

      spdlog::trace("Setting I2C device address to 0x{:02X} of fd={}", addr, this->_fd);

      if (::ioctl(this->_fd, I2C_SLAVE, addr) < 0) {
        spdlog::error("Failed to set I2C device address to 0x{:02X} on {}: fd={} "
                      "| error={}",
                      addr,
                      this->_bus,
                      this->_fd,
                      strerror(errno));
        throw std::runtime_error("Failed to set I2C device address");
      }

      this->_activeAddr = static_cast<int>(addr);
    }

  private:
    std::string _bus;
    int _fd{-1};
    mutable int _activeAddr{-1};
  };

  class Device {
  public:
    Device(Bus &bus, std::string name, uint8_t addr) :
        _bus(bus), _name(std::move(name)), _addr(addr) {}

    [[nodiscard]] const std::string &name() const { return this->_name; }

    [[nodiscard]] uint8_t readByte(uint8_t reg) const {
      this->_bus.setAddress(this->_addr);

      if (::write(this->_bus.fd(), &reg, 1) != 1) {
        spdlog::error("Failed to write register address 0x{:02X} to I2C device "
                      "{} at address 0x{:02X}: {}",
                      reg,
                      this->_name,
                      this->_addr,
                      strerror(errno));
        throw std::runtime_error("Failed to write register address to I2C device");
      }

      uint8_t value = 0;

      if (::read(this->_bus.fd(), &value, 1) != 1) {
        spdlog::error("Failed to read byte from I2C device {} at address "
                      "0x{:02X}, register 0x{:02X}: {}",
                      this->_name,
                      this->_addr,
                      reg,
                      strerror(errno));
        throw std::runtime_error("Failed to read byte from I2C device");
      }

      return value;
    }

    [[nodiscard]] int16_t readShort(uint8_t loReg, uint8_t hiReg) const {
      const uint8_t low = this->readByte(loReg);
      const uint8_t high = this->readByte(hiReg);

      const uint16_t value = (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);

      return static_cast<int16_t>(value);
    }

    void writeByte(uint8_t reg, uint8_t value) const {
      this->_bus.setAddress(this->_addr);

      const std::array<uint8_t, 2> buffer = {reg, value};

      if (::write(this->_bus.fd(), buffer.data(), 2) != 2) {
        spdlog::error("Failed to write byte to I2C device {} at address "
                      "0x{:02X}, register 0x{:02X}: {}",
                      this->_name,
                      this->_addr,
                      reg,
                      strerror(errno));
        throw std::runtime_error("Failed to write byte to I2C device");
      }
    }

  private:
    Bus &_bus;
    std::string _name;
    uint8_t _addr;
  };
} // namespace i2c
