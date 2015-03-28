#ifndef CEC_FORWARD_LIST
#define CEC_FORWARD_LIST

#include <forward_list>
#include <cec/extended_sequence_container.hpp>

namespace cec {

/**
 * @brief A convenience alias for extended std::forward_list
 */
template <typename T, typename Allocator = std::allocator<T>>
using forward_list =
    cec::extended_sequence_container<std::forward_list<T, Allocator>>;
}

#endif
