#include <csignal>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <chrono>

#include <spdlog/spdlog.h>

#include "sense_hat.hpp"
#include "timer.hpp"

void pollSenseHat(SenseHat &senseHat)
{
  float tempC = senseHat.readTemperature();
  float tempF = senseHat.readTemperature(true);
  spdlog::info("Current temperature: {:.2f}°C / {:.2f}°F", tempC, tempF);
}

namespace
{
  std::atomic<bool> shouldExit{false};

  void signalHandler(int /*signal*/) noexcept
  {
    shouldExit.store(true, std::memory_order_relaxed);
  }
}

int main()
{
  spdlog::set_level(spdlog::level::debug);
  spdlog::info("Starting Sense application...");

  SenseHat senseHat;
  senseHat.testHardware();

  Timer timer([&]()
              { pollSenseHat(senseHat); }, 1000);

  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGHUP, signalHandler);
  std::signal(SIGQUIT, signalHandler);
  std::signal(SIGABRT, signalHandler);
  std::signal(SIGSEGV, signalHandler);
  std::signal(SIGFPE, signalHandler);
  std::signal(SIGILL, signalHandler);

  timer.start();

  while (!shouldExit.load(std::memory_order_relaxed))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  timer.stop();

  spdlog::info("Exit requested, stopping timer...");
  timer.stop();

  spdlog::info("Sense application closed");
  return 0;
}
