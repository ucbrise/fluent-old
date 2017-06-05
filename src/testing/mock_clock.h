#ifndef TESTING_MOCK_CLOCK_H_
#define TESTING_MOCK_CLOCK_H_

#include <cstdint>

#include <chrono>
#include <ratio>

namespace fluent {

// # Usage
//
//   MockClock::now(); // 0 seconds since the epoch
//   MockClock::Advance(std::chrono::seconds(1))
//   MockClock::now(); // 1 second since epoch
//   MockClock::Advance(std::chrono::seconds(1))
//   MockClock::now(); // 2 second since epoch
//   MockClock::Reset()
//   MockClock::now(); // 0 seconds since the epoch
//   MockClock::Advance(std::chrono::seconds(1))
//   MockClock::now(); // 1 second since epoch
//
// # Motiviation
// MockClock is an implementation of the Clock concept [1] that can be used for
// dependency injection. For example, consider the following class which logs
// messages:
//
//   struct Logger {
//     std::string format(const std::string& msg) const {
//       return "[" + std::system_clock::now() + "] " + msg;
//     }
//
//     void log(const std::string& msg) const {
//       std::cout << format(msg) << std::endl;
//     }
//   };
//
// Writing a unit test for this class is challenging because we don't know what
// std::system_clock::now() is going to return. To overcome this challenge, we
// parameterize Logger like this:
//
//   template <typename Clock>
//   class Logger {
//     std::string format(const std::string& msg) const {
//       return "[" + std::system_clock::now() + "] " + msg;
//     }
//
//     void log(const std::string& msg) const {
//       std::cout << format(msg) << std::endl;
//     }
//   };
//
// When we use Logger for real, we'll use a Logger<std::system_clock>. When we
// want to unit test Logger, we'll use a Logger<MockClock>.
//
// [1]: http://en.cppreference.com/w/cpp/concept/Clock
class MockClock {
 public:
  using rep = std::chrono::system_clock::rep;
  using period = std::chrono::system_clock::period;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<MockClock>;

  static constexpr bool is_steady = false;

  template <typename Rep, typename Period>
  static void Advance(const std::chrono::duration<Rep, Period>& d) {
    now_ += d;
  }

  static void Reset();

  static time_point now();

 private:
  MockClock() = delete;

  static time_point now_;
};

}  // namespace fluent

#endif  // TESTING_MOCK_CLOCK_H_
