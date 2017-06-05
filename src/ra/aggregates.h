#ifndef RA_AGGREGATES_H_
#define RA_AGGREGATES_H_

#include <cstddef>

#include "fmt/format.h"

namespace fluent {
namespace ra {
namespace agg {

// The GroupBy class is paramaterized on a variadic number of aggregates. This
// allows us to express the following SQL query
//
//   SELECT R.a, R.b, SUM(R.c), AVG(R.d), COUNT(R.e)
//   FROM R
//   GROUP BY R.a, R.b
//
// in C++
//
//   make_iterable(r) | group_by<Keys<0, 1>, Sum<2>, Avg<3>, Count<4>>();
//
// Each of those aggregates should look like this:
//
//   template <std::size_t, typename T>
//   class AggregateImpl {
//    public:
//     void Update(const T& x) { ... }
//     U Get() const { ... }
//   };
//
//   template <std::size_t I>
//   struct Aggregate {
//     template <typename T>
//     using type = AggregateImpl<I, T>;
//
//     static std::string ToDebugString const { ... }
//   };
//
// Each aggregate (e.g. Sum, Avg, Count) is paramaterized on a size_t denoting
// the column over which the aggregate will be applied. The struct contains a
// single type, called `type`, that is paramaterized on the column `I` and a
// type `T`. `T` will be instantiated with the type of the `I`th column.
//
// The aggregate implementation (e.g. SumImpl, AvgImpl) has a method Update
// which takes in values of the column, and a method `Get` which returns the
// final aggregate. The return type of Get is arbitrary.

template <std::size_t, typename T>
class SumImpl {
 public:
  SumImpl() : sum_() {}
  void Update(const T& x) { sum_ += x; }
  T Get() const { return sum_; }

 private:
  T sum_;
};

template <std::size_t I>
struct Sum {
  template <typename T>
  using type = SumImpl<I, T>;

  static std::string ToDebugString() { return fmt::format("Sum<{}>", I); }
};

template <std::size_t, typename T>
class CountImpl {
 public:
  void Update(const T&) { count_++; }
  std::size_t Get() const { return count_; }

 private:
  std::size_t count_ = 0;
};

template <std::size_t I>
struct Count {
  template <typename T>
  using type = CountImpl<I, T>;

  static std::string ToDebugString() { return fmt::format("Count<{}>", I); }
};

template <std::size_t, typename T>
class AvgImpl {
 public:
  void Update(const T& x) {
    sum_ += x;
    count_++;
  }

  double Get() const { return sum_ / count_; }

 private:
  double sum_ = 0;
  std::size_t count_ = 0;
};

template <std::size_t I>
struct Avg {
  template <typename T>
  using type = AvgImpl<I, T>;

  static std::string ToDebugString() { return fmt::format("Avg<{}>", I); }
};

}  // namespace agg
}  // namespace ra
}  // namespace fluent

#endif  //  RA_AGGREGATES_H_
