#ifndef TESTING_CAPTURED_STDOUT_H_
#define TESTING_CAPTURED_STDOUT_H_

#include <iostream>
#include <sstream>
#include <string>

namespace fluent {
namespace testing {
// CapturedStdout can be used to capture contents written to stdout.
//
//   {
//     CapturedStdout captured;                      // cout is now captured.
//     std::cout << "hello" << std:endl;             // Write to cout as normal.
//     EXPECT_STREQ("hello\n", captured.Get().str()) // Get captured cout.
//   }
//   // When captured goes out of scope, cout is no longer captured. The
//   // following will be printed.
//   std::cout << "yoloswag" << std::endl;
class CapturedStdout {
 public:
  CapturedStdout() : cout_sbuf_(std::cout.rdbuf()) {
    std::cout.rdbuf(buffer_.rdbuf());
  }

  ~CapturedStdout() { std::cout.rdbuf(cout_sbuf_); }

  std::string Get() { return buffer_.str(); }

 private:
  std::stringstream buffer_;
  std::streambuf* cout_sbuf_;
};

}  // namespace testing
}  // namespace fluent

#endif  // TESTING_CAPTURED_STDOUT_H_
