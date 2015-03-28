#ifndef CEC_VECTOR
#define CEC_VECTOR

#include <vector>
#include <cec/extended_sequence_container.hpp>

namespace cec {

/**
 * @brief A convenience alias for extended std::vector
 */
template <typename T, typename Allocator = std::allocator<T>>
using vector = cec::extended_sequence_container<std::vector<T, Allocator>>;
}

#endif
