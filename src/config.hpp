#pragma once

#include <expected>
#include <stdexcept>
#include <string>
#include <system_error>
#include "ini_manager.hpp"
#include "spdlog/common.h"
#include <spdlog/spdlog.h>

struct SensorConfig {
  bool RunHealthCheckOnStartup;
};

struct LoggerConfig {
  spdlog::level::level_enum LogLevel;
};

struct ExporterConfig {
  bool Enabled;
};

class Config {
public:
  SensorConfig Sensor{};
  LoggerConfig Logger{};
  ExporterConfig Exporter{};

  Config(const std::string &configFilePath) {
    const std::expected<ini::ini_manager, std::error_code> configResult = ini::ini_manager::from_file(configFilePath);

    if (!configResult.has_value()) {
      throw std::runtime_error("Failed to load configuration: " + configResult.error().message());
    }

    const ini::ini_manager &config = configResult.value();

    this->Sensor.RunHealthCheckOnStartup = config.get_value_or_default(ini::section{"Sensor"},
                                                                       ini::key{"RunHealthCheckOnStartup"},
                                                                       false);

    this->Logger.LogLevel = spdlog::level::from_str(
        config.get_value_or_default<std::string>(ini::section{"Logger"}, ini::key{"LogLevel"}, "info"));

    this->Exporter.Enabled = config.get_value_or_default(ini::section{"Exporter"}, ini::key{"Enabled"}, false);
  }
};
