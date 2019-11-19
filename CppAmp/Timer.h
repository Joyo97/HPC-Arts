#pragma once

#include <chrono>

class Timer
{
  using clock = std::chrono::high_resolution_clock;
  using msec = std::chrono::milliseconds;
  clock::time_point start_;
public:
  void Reset()
  {
    start_ = clock::now();
  }

  msec Elapsed() const
  {
    return std::chrono::duration_cast<msec>(clock::now() - start_);
  }

  explicit Timer(bool run = false)
  {
    if (run) Reset();
  }

  template<typename T, typename Traits>
  friend basic_ostream<T, Traits>& operator<<(basic_ostream<T, Traits>& out, const Timer& timer)
  {
    return out << timer.Elapsed().count();
  }
};