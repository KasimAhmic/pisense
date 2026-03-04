#pragma once

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define TRY(expr)                                                                                                      \
  ({                                                                                                                   \
    auto _tmp_##__LINE__ = (expr);                                                                                     \
    if (!_tmp_##__LINE__) {                                                                                            \
      return std::unexpected(_tmp_##__LINE__.error());                                                                 \
    }                                                                                                                  \
    std::move(_tmp_##__LINE__).value();                                                                                \
  })

namespace i2c {
  class Bus {
  public:
    explicit Bus(std::string bus) :
        _bus(std::move(bus)) {

      this->_fd = ::open(this->_bus.c_str(), O_RDWR);

      if (this->_fd < 0) {
        throw std::runtime_error("Failed to open I2C bus");
      }
    }

    ~Bus() noexcept {
      if (this->_fd >= 0) {

        if (::close(this->_fd) < 0) {
          // TODO: Figure out what to do here...
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

    std::expected<void, std::error_code> setAddress(uint8_t addr) const {
      if (this->_activeAddr == static_cast<int>(addr)) {
        return {};
      }

      if (::ioctl(this->_fd, I2C_SLAVE, addr) < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
      }

      this->_activeAddr = static_cast<int>(addr);

      return {};
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

    virtual std::vector<std::pair<std::string, bool>> test() = 0;

  protected:
    template <size_t N>
    [[nodiscard]] std::expected<std::array<uint8_t, N>, std::error_code> readBytes(uint8_t reg) const {
      TRY(this->_bus.setAddress(this->_addr));

      std::array<uint8_t, N> buffer{};
      uint8_t startReg = reg | 0x80; // TODO: Make this some constant somewhere

      std::array<i2c_msg, 2> messages{
          {{.addr = this->_addr, .flags = 0, .len = static_cast<uint16_t>(1), .buf = &startReg},
           {.addr = this->_addr, .flags = I2C_M_RD, .len = static_cast<uint16_t>(N), .buf = buffer.data()}},
      };

      const i2c_rdwr_ioctl_data packets{.msgs = messages.data(), .nmsgs = static_cast<uint32_t>(messages.size())};

      if (ioctl(this->_bus.fd(), I2C_RDWR, &packets) < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
      }

      return buffer;
    }

    [[nodiscard]] std::expected<uint8_t, std::error_code> readByte(uint8_t reg) const {
      const auto result = this->readBytes<1>(reg);

      if (!result) {
        return std::unexpected(result.error());
      }

      return (*result)[0];
    }

    [[nodiscard]] std::expected<int16_t, std::error_code> readShort(uint8_t reg) const {
      const auto data = this->readBytes<2>(reg);

      if (!data) {
        return std::unexpected(data.error());
      }

      const std::array<uint8_t, 2> &bytes = *data;

      const uint16_t combined = (uint16_t(bytes[1]) << 8) | uint16_t(bytes[0]);
      return static_cast<int16_t>(combined);
    }

    [[nodiscard]] std::expected<void, std::error_code> writeByte(uint8_t reg, uint8_t value) const {
      TRY(this->_bus.setAddress(this->_addr));

      const std::array<uint8_t, 2> buffer = {reg, value};

      if (::write(this->_bus.fd(), buffer.data(), 2) != 2) {
        return std::unexpected(std::error_code(errno, std::system_category()));
      }

      return {};
    }

  private:
    Bus &_bus;
    std::string _name;
    uint8_t _addr;
  };
} // namespace i2c
