#include "testing/mock_clock.h"

namespace fluent {
namespace testing {

MockClock::time_point MockClock::now_;

void MockClock::Reset() { now_ = time_point(); }

MockClock::time_point MockClock::now() { return now_; }

}  // namespace testing
}  // namespace fluent
