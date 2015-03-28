#ifndef CEC_DEQUE
#define CEC_DEQUE

#include <deque>
#include <cec/extended_sequence_container.hpp>

namespace cec {

/**
 * @brief A convenience alias for extended std::deque
 */
template <typename T, typename Allocator = std::allocator<T>>
using deque = cec::extended_sequence_container<std::deque<T, Allocator>>;
}

#endif
