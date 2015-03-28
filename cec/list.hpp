#ifndef CEC_LIST
#define CEC_LIST

#include <list>
#include <cec/extended_sequence_container.hpp>

namespace cec {

/**
 * @brief A convenience alias for extended std::list
 */
template <typename T, typename Allocator = std::allocator<T>>
using list = cec::extended_sequence_container<std::list<T, Allocator>>;
}

#endif
