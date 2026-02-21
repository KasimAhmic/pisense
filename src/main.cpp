#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#include <spdlog/spdlog.h>

#include "sense_hat.hpp"
#include "timer.hpp"

constexpr uint32_t POLL_INTERVAL_MS = 1000;
constexpr uint32_t EXIT_CHECK_INTERVAL_MS = 500;

void pollSenseHat(SenseHat &senseHat) {
  float tempC = senseHat.readTemperature();
  float tempF = senseHat.readTemperature(true);
  spdlog::info("Current temperature: {:.2f}°C / {:.2f}°F", tempC, tempF);
}

namespace {
  std::atomic<bool> shouldExit{false};

  void signalHandler(int /*signal*/) noexcept { shouldExit.store(true, std::memory_order_relaxed); }
} // namespace

int main() {
  spdlog::set_level(spdlog::level::debug);
  spdlog::info("Starting Sense application...");

  SenseHat senseHat;
  senseHat.testHardware();

  Timer timer([&]() { pollSenseHat(senseHat); }, POLL_INTERVAL_MS);

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGHUP, signalHandler);
  std::signal(SIGQUIT, signalHandler);
  std::signal(SIGABRT, signalHandler);
  std::signal(SIGSEGV, signalHandler);
  std::signal(SIGFPE, signalHandler);
  std::signal(SIGILL, signalHandler);

  timer.start();

  while (!shouldExit.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(EXIT_CHECK_INTERVAL_MS));
  }

  timer.stop();

  spdlog::info("Exit requested, stopping timer...");
  timer.stop();

  spdlog::info("Sense application closed");
  return 0;
}
