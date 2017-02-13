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

}  // namespace fluent

#endif  // COMMON_MACROS_H
