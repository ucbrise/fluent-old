#ifndef COMMON_FUNCTION_TRAITS_H_
#define COMMON_FUNCTION_TRAITS_H_

namespace fluent {

template <typename T>
class has_Iterable
{
    typedef char one;
    typedef long two;

    template <typename C> static one test( decltype(&C::Iterable) ) ;
    template <typename C> static two test(...);    

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// template <typename T>  
// struct has_Iterable
// {
//     struct dummy { /* something */ };

//     template <typename C, typename P>
//     static auto test(P * p) -> decltype(std::declval<C>().Iterable(*p), std::true_type());

//     template <typename, typename>
//     static std::false_type test(...);

//     typedef decltype(test<T, dummy>(nullptr)) type;
//     static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
// };

}  // namespace fluent

#endif  //  COMMON_FUNCTION_TRAITS_H_