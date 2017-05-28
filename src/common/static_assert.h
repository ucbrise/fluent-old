#ifndef COMMON_STATIC_ASSERT_H
#define COMMON_STATIC_ASSERT_H

namespace fluent {

// Consider the struct template which statically asserts that the type
// parameters B and E are the same.
//
//   template <typename A, typename B, typename C, typename D, typename E>
//   struct S {
//     static_assert(std::is_same<B, E>::value, "B != E");
//   };
//
// Next, imagine that we instantiated the struct template with some really long
// type parameters:
//
//   using A = std::map<std::set<std::set<int>>, std::set<std::set<int>>>>;
//   using B = std::map<std::set<std::tuple<bool>>, std::set<std::set<bool>>>>;
//   using C = std::map<std::set<std::set<char>>, std::set<std::tuple<char>>>>;
//   using D = std::map<std::set<std::set<float>>, std::set<std::set<float>>>>;
//   using E = std::map<std::set<std::tuple<long>>, std::set<std::set<long>>>>;
//   S<A, B, C, D, E> s;
//
// Here, B != E, so the static assert inside of S will fail. The resulting
// error message is really hard to read:
//
//   error: static_assert failed "B != E"
//     static_assert(std::is_same<B, E>::value, "B != E");
//   note: in instantiation of template class
//   'S<std::map<std::set<std::set<int>>, std::set<std::set<int>>>>,
//   std::map<std::set<std::tuple<bool>>, std::set<std::set<bool>>>>,
//   std::map<std::set<std::set<char>>, std::set<std::tuple<char>>>>,
//   std::map<std::set<std::set<float>>, std::set<std::set<float>>>>,
//   std::map<std::set<std::tuple<long>>, std::set<std::set<long>>>>>'
//
// Where does one type parameter end? Where does the next start? Which one is
// B? Which one is E? It's hard to tell. The StaticAssert struct makes it a bit
// easier to find the relevant type parameters in a static assert. You use it
// like this:
//
//   template <typename A, typename B, typename C, typename D, typename E>
//   struct S {
//     static_assert(StaticAssert<std::is_same<B, E>>::value, "B != E");
//   };
//
// Now, instantiating S will produce the following error message:
//
//   error: static_assert failed "See the error messages below and look for the
//                                StaticAssert template to see what assertion
//                                failed."
//   note: in instantiation of template class
//   'fluent::StaticAssert<std::is_same<std::map<std::set<std::tuple<bool>>,
//   std::set<std::set<bool>>>>, std::map<std::set<std::tuple<long>>,
//   std::set<std::set<long>>>>>>'
//
// Still not super easy to read, but it's a bit easier.
template <typename Assertion>
struct StaticAssert {
  static_assert(Assertion::value,
                "See the error messages below and look for the StaticAssert "
                "template to see what assertion failed.");
  static constexpr bool value = Assertion::value;
};

}  // namespace fluent

#endif  // COMMON_STATIC_ASSERT_H
