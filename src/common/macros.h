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

// DEFAULT_MOVE_AND_ASSIGN is a macro you can use to add a default move
// constructor and default move assignment operator to a class. Now, you might
// be thinking to yourself, doesn't a class have a *default* move constructor
// and *default* move assignment operator, well, by *default*! Yes, it does. In
// some circumstances. For example, the following code works just fine:
//
//   struct Foo {
//     Foo() {}
//   }
//
//   Foo f1;
//   Foo f2(f1);            // default copy constructor
//   Foo f3(std::move(f1)); // default move constructor
//
// However, in other circumstances, it does not. For example, imagine we
// explicitly delete the copy constructor of Foo.
//
//   struct Foo {
//     Foo() {}
//     DISALLOW_COPY_AND_ASSIGN(Foo);
//   }
//
// As expected, this removes the copy constructor. But, it also removes the
// move constructor:
//
//   Foo f1;
//   Foo f2(f1);            // does not compile!
//   Foo f3(std::move(f1)); // does not compile!
//
// We can use the DEFAULT_MOVE_AND_ASSIGN macro to make Foo movable again.
//
//   struct Foo {
//     Foo() {}
//     DISALLOW_COPY_AND_ASSIGN(Foo);
//     DEFAULT_MOVE_AND_ASSIGN(Foo);
//   }
//
//   Foo f1;
//   Foo f2(f1);            // does not compile!
//   Foo f3(std::move(f1)); // default move constructor
#define DEFAULT_MOVE_AND_ASSIGN(TypeName) \
  TypeName(TypeName&&) = default;         \
  TypeName& operator=(TypeName&&) = default;

// The following code produces an unused variable warning:
//
//   int x = 0;
//
// Wrapping x with the UNUSED macros silences this warning:
//
//   int x = 0;
//   UNUSED(x);
#define UNUSED(x) \
  do {            \
    (void)(x);    \
  } while (0)

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
