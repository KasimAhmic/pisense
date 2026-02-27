#pragma once

#include <cstdint>
#include <expected>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "ini_manager.hpp"
#include "spdlog/common.h"
#include <spdlog/spdlog.h>

#define ReadConfig(type, section, keyName) config.get_value<type>(section, ini::key{keyName})
#define ReadString(section, keyName) ReadConfig(std::string, section, keyName);
#define ReadDouble(section, keyName) ReadConfig(double, section, keyName);
#define ReadBool(section, keyName) ReadConfig(bool, section, keyName);
#define ReadUInt32(section, keyName) ReadConfig(uint32_t, section, keyName);

struct AppConfig {
  uint32_t PollingIntervalMs;
  uint32_t ExitCheckIntervalMs;
};

struct HTS221Config {
  enum class TemperatureCompensationMode : uint8_t { None, Simple, Linear, Cpu };

  TemperatureCompensationMode TemperatureCompensationMode;
  double SimpleCompensationTemperatureOffset;
  double LinearCompensationTemperatureScale;
  double LinearCompensationTemperatureOffset;
  double CpuCompensationCpuCoefficient;

  [[nodiscard]] static enum TemperatureCompensationMode toTempCompMode(const std::string &modeStr) {
    if (modeStr == "None") {
      return TemperatureCompensationMode::None;
    }

    if (modeStr == "Simple") {
      return TemperatureCompensationMode::Simple;
    }

    if (modeStr == "Linear") {
      return TemperatureCompensationMode::Linear;
    }

    if (modeStr == "Cpu") {
      return TemperatureCompensationMode::Cpu;
    }

    spdlog::warn("Invalid TemperatureCompensationMode '{}', defaulting to 'None'", modeStr);
    return TemperatureCompensationMode::None;
  };
};

struct LoggerConfig {
  spdlog::level::level_enum LogLevel;
};

struct ExporterConfig {
  bool Enabled;
};

struct DebugConfig {
  bool PrintConfigOnStartup;
  bool RunHealthCheckOnStartup;
};

class Config {
public:
  AppConfig App{};
  HTS221Config HTS221{};
  LoggerConfig Logger{};
  ExporterConfig Exporter{};
  DebugConfig Debug{};

  Config(const std::string &configFilePath) {
    const std::expected<ini::ini_manager, std::error_code> configResult = ini::ini_manager::from_file(configFilePath);

    if (!configResult.has_value()) {
      throw std::runtime_error("Failed to load configuration: " + configResult.error().message());
    }

    const ini::ini_manager &config = configResult.value();

    // App Section
    {
      const auto app = ini::section{Config::APP_SECTION};

      const auto pollingInterval = ReadUInt32(app, "PollingIntervalMs");
      const auto exitCheckInterval = ReadUInt32(app, "ExitCheckIntervalMs");

      this->App.PollingIntervalMs = pollingInterval.value_or(1000);
      this->App.ExitCheckIntervalMs = exitCheckInterval.value_or(500);
    }

    // HTS221 Section
    {
      const auto hts221 = ini::section{Config::HTS221_SECTION};

      const auto tempCompMode = ReadString(hts221, "TemperatureCompensationMode");
      const auto simpleCompTempOffset = ReadDouble(hts221, "SimpleCompensationTemperatureOffset");
      const auto linearCompTempScale = ReadDouble(hts221, "LinearCompensationTemperatureScale");
      const auto linearCompTempOffset = ReadDouble(hts221, "LinearCompensationTemperatureOffset");
      const auto cpuCompCpuCoefficient = ReadDouble(hts221, "CpuCompensationCpuCoefficient");

      this->HTS221.TemperatureCompensationMode = HTS221Config::toTempCompMode(tempCompMode.value_or("None"));
      this->HTS221.SimpleCompensationTemperatureOffset = simpleCompTempOffset.value_or(0.0);
      this->HTS221.LinearCompensationTemperatureScale = linearCompTempScale.value_or(0.0);
      this->HTS221.LinearCompensationTemperatureOffset = linearCompTempOffset.value_or(0.0);
      this->HTS221.CpuCompensationCpuCoefficient = cpuCompCpuCoefficient.value_or(0.0);
    }

    // Logger Section
    {
      const auto logger = ini::section{Config::LOGGER_SECTION};

      const auto logLevel = ReadString(logger, "LogLevel");

      this->Logger.LogLevel = spdlog::level::from_str(logLevel.value_or("info"));
    }

    // Exporter Section
    {
      const auto exporter = ini::section{Config::EXPORTER_SECTION};

      const auto exporterEnabled = ReadBool(exporter, "Enabled");

      this->Exporter.Enabled = exporterEnabled.value_or(false);
    }

    // Debug Section
    {
      const auto debug = ini::section{Config::DEBUG_SECTION};

      const auto printConfig = ReadBool(debug, "PrintConfigOnStartup");
      const auto runHealthCheck = ReadBool(debug, "RunHealthCheckOnStartup");
      const auto printAllHealthCheckResults = ReadBool(debug, "PrintAllHealthCheckResults");

      this->Debug.PrintConfigOnStartup = printConfig.value_or(false);
      this->Debug.RunHealthCheckOnStartup = runHealthCheck.value_or(false);
    }
  }

private:
  static constexpr std::string APP_SECTION = "App";
  static constexpr std::string HTS221_SECTION = "HTS221";
  static constexpr std::string LOGGER_SECTION = "Logger";
  static constexpr std::string EXPORTER_SECTION = "Exporter";
  static constexpr std::string DEBUG_SECTION = "Debug";

  static void createDefaultConfigFile(const std::string &filePath) {
    ini::ini_manager defaultConfig;
    defaultConfig.set_section(Config::APP_SECTION);
    defaultConfig.set_value(Config::APP_SECTION, "Once", "false");
    defaultConfig.set_value(Config::APP_SECTION, "PollingIntervalMs", "1000");
    defaultConfig.set_value(Config::APP_SECTION, "ExitCheckIntervalMs", "500");

    defaultConfig.set_section(Config::DEBUG_SECTION);
    defaultConfig.set_value(Config::DEBUG_SECTION, "RunHealthCheckOnStartup", "false");

    defaultConfig.set_section(Config::LOGGER_SECTION);
    defaultConfig.set_value(Config::LOGGER_SECTION, "LogLevel", "info");

    defaultConfig.set_section(Config::EXPORTER_SECTION);
    defaultConfig.set_value(Config::EXPORTER_SECTION, "Enabled", "false");

    std::ofstream configFile(filePath);

    if (configFile.is_open()) {
      configFile << defaultConfig;
      configFile.close();
      spdlog::info("Created default configuration file at '{}'", filePath);
    } else {
      spdlog::error("Failed to create default configuration file at '{}'", filePath);
    }
  }
};

#undef ReadString
#undef ReadDouble
#undef ReadBool
#undef ReadUInt32
