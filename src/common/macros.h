#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

namespace fluent {

// DISALLOW_COPY_AND_ASSIGN is a macro which you can use to make a class
// non-copyable and non-copy-assignable. For example, imagine you have a class
// which contains a vector.
//
//   class CopyableAndAssignable {
//    public:
//     CopyableAndAssignable(std::vector<int> xs) : xs_(std::move(xs)) {}
//    private:
//     std::vector<int> xs_;
//   };
//
// This class is both copyable and assignable. For example, the following code
// will compile just fine:
//
//   CopyableAndAssignable x({1, 2, 3});
//   CopyableAndAssignable y(x);
//   y = x;
//
// If we want to avoid accidental copies and make the class non-copyable and
// non-assignable, we can do so by adding the macro invocation
// DISALLOW_COPY_AND_ASSIGN(ClassName) somewhere in the class:
//
//   class NotCopyableAndAssignable {
//    public:
//     NotCopyableAndAssignable(std::vector<int> xs) : xs_(std::move(xs)) {}
//     DISALLOW_COPY_AND_ASSIGN(NotCopyableAndAssignable)
//    private:
//     std::vector<int> xs_;
//   };
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete;

// Prepending a function declaration with the WARN_UNUSED macro will cause the
// compiler to emit a warning if the return value is not used. For example, the
// following code will emit a warning because the return of f is not used.
//
//   WARN_UNUSED int f(int x) {
//     return x;
//   }
//
//   int main() {
//     f(42); // Warning.
//   }
//
// This code was taken from http://stackoverflow.com/a/2043239/3187068 and
// works with both g++ and clang++.
#define WARN_UNUSED __attribute__((warn_unused_result))

}  // namespace fluent

#endif  // COMMON_MACROS_H
