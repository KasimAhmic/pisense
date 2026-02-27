#pragma once

#include <atomic>
#include <chrono>
#include <print>
#include <string_view>
#include <thread>

#include <json.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "sense_hat.hpp"
#include "timer.hpp"

using nlohmann::json;

class PiSense {
public:
  PiSense(Config config, const std::atomic<bool> &shouldExit, const std::atomic<int> &exitSig) :
      _config(config), _shouldExit(shouldExit), _exitSignal(exitSig) {}

  ~PiSense() = default;

  int run(bool once) {
    spdlog::set_level(once ? spdlog::level::off : this->_config.Logger.LogLevel);

    spdlog::info("Starting Sense application...");

    if (once) {
      spdlog::info("Running once...");
      this->tick();
      return 0;
    }

    if (this->_config.Debug.RunHealthCheckOnStartup) {
      spdlog::info("Running health check...");
      this->_senseHat.testHardware();
    }

    Timer timer([&]() { this->tick(); }, this->_config.App.PollingIntervalMs);
    timer.start();

    while (!this->shouldClose()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(this->_config.App.ExitCheckIntervalMs));
    }

    std::println(); // So the exit message doesn't appear on the same line as the control character
    spdlog::warn("Exiting... (signal: {})", this->getExitSignal());

    timer.stop();

    spdlog::info("Sense application closed");

    return 0;
  }

private:
  struct SpdLogger {
    static void trace(std::string_view msg) { spdlog::trace(msg); }
    static void debug(std::string_view msg) { spdlog::debug(msg); }
    static void info(std::string_view msg) { spdlog::info(msg); }
    static void warn(std::string_view msg) { spdlog::warn(msg); }
    static void error(std::string_view msg) { spdlog::error(msg); }
    static void critical(std::string_view msg) { spdlog::critical(msg); }
  };

  Config _config;
  SenseHat<SpdLogger> _senseHat;
  const std::atomic<bool> &_shouldExit;
  const std::atomic<int> &_exitSignal;

  bool shouldClose() const { return _shouldExit.load(std::memory_order_relaxed); }
  int getExitSignal() const { return _exitSignal.load(std::memory_order_relaxed); }

  void tick() const {
    const double tempC = this->_senseHat.readTemperature();
    const double tempF = (tempC * (9.0 / 5.0)) + 32.0;
    const double humidity = this->_senseHat.readHumidity();

    const json::object_t output{
        {"temperature_celsius", tempC},
        {"temperature_fahrenheit", tempF},
        {"humidity", humidity},
    };

    std::println("{}", json(output).dump());
  }
};
