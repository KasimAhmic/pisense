#include <atomic>
#include <csignal>
#include <cstdio>
#include <string>

#include <argparse.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "pisense.hpp"

namespace {
  std::atomic<bool> shouldExit{false};
  std::atomic<int> exitSignal{0};

  void signalHandler(int signal) noexcept {
    shouldExit.store(true, std::memory_order_relaxed);
    exitSignal.store(signal, std::memory_order_relaxed);
  }
} // namespace

int main(int argc, const char *argv[]) {
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGHUP, signalHandler);
  std::signal(SIGQUIT, signalHandler);

  std::setvbuf(stdout, nullptr, _IOLBF, 0);

  argparse::ArgumentParser program("pisense", "1.0.0");
  program.add_description("A Raspberry Pi application for reading sensor data from a Raspberry Pi Sense HAT.");
  program.add_epilog("Created by Kasim Ahmic. Source code available at https://github.com/KasimAhmic/pisense");

  program.add_argument("--once", "-o")
      .help("polls the SenseHat one time, outputs the data, and exits")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--config", "-c")
      .help("path to the configuration file")
      .default_value("config.ini")
      .implicit_value(false);

  program.parse_args(argc, argv);

  const bool once = program.get<bool>("--once");

  if (once) {
    spdlog::set_level(spdlog::level::off);
  }

  Config config(program.get<std::string>("--config"));

  PiSense app(config, shouldExit, exitSignal);

  return app.run(once);
};
