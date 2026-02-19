#pragma once

#include <functional>
#include <thread>
#include <condition_variable>

class Timer
{
public:
  Timer(const std::function<void()> &callback, const uint32_t interval) : callback(callback),
                                                                          interval(interval),
                                                                          running(false) {}
  ~Timer()
  {
    this->stop();
  }

  void start()
  {
    if (this->running.load())
    {
      return;
    }

    this->running.store(true);

    this->worker = std::thread([this]
                               {
        std::unique_lock lock(this->mutex);

        while (this->running.load()) {
            this->trigger.wait_for(lock, std::chrono::milliseconds(this->interval), [this] {
                return !this->running.load();
            });

            this->callback();
        } });
  }

  void stop()
  {
    this->running.store(false);
    this->trigger.notify_all();

    if (this->worker.joinable())
    {
      this->worker.join();
    }
  }

private:
  std::function<void()> callback;
  uint32_t interval;
  std::atomic<bool> running;
  std::thread worker;
  std::condition_variable trigger;
  std::mutex mutex;
};
