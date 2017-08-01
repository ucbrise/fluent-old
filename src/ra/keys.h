#ifndef RA_COMMON_H
#define RA_COMMON_H

#include <cstddef>

#include "common/sizet_list.h"

namespace fluent {
namespace ra {

template <std::size_t... Is>
using Keys = common::SizetList<Is...>;

template <std::size_t... Is>
using LeftKeys = common::SizetList<Is...>;

template <std::size_t... Is>
using RightKeys = common::SizetList<Is...>;

}  // namespace ra
}  // namespace fluent

#endif  // RA_COMMON_H
