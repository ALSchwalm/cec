#ifndef CEC_EXTENDED_SEQUENCE_CONTAINER_DETAL
#define CEC_EXTENDED_SEQUENCE_CONTAINER_DETAL

#include <memory>
#include <iterator>
#include <tuple>

namespace cec {
namespace detail {

// If matches==true, 'type' is Allocator rebind_alloc with the type Rebind,
// otherwise, it is just Allocator
template <bool Matches, typename Allocator, typename Rebind>
struct rebind_alloc_if_matches_helper {
    using type = typename std::allocator_traits<
        Allocator>::template rebind_alloc<Rebind>;
};

// This is not the type we're looking for, ignore it
template <typename T, typename Rebind>
struct rebind_alloc_if_matches_helper<false, T, Rebind> {
    using type = T;
};

// If the type of Head is the type of Allocator, rebind
// Allocator to the Rebind type
template <typename Head, typename Allocator, typename Rebind>
struct rebind_alloc_if_matches
    : rebind_alloc_if_matches_helper<std::is_same<Head, Allocator>::value,
                                     Allocator, Rebind> {};

// Declaration
template <typename Container, typename RebindType, typename Unchecked,
          typename Checked>
struct rebind_sequence_container_helper;

// Base case. If all types have been checked (allocators have been rebound)
// then type is the container with the first parameter changed to 'Rebind'
template <template <typename...> class Container, typename Rebind,
          typename... Checked, typename... Args, typename Old>
struct rebind_sequence_container_helper<
    Container<Args...>, Rebind, std::tuple<>, std::tuple<Old, Checked...>> {
    using type = Container<Rebind, Checked...>;
};

// Loop. Check the head of the type pack, if it is the allocator type for
// Container, rebind it to the new type.
template <typename Container, typename Rebind, typename Head,
          typename... Unchecked, typename... Checked>
struct rebind_sequence_container_helper<
    Container, Rebind, std::tuple<Head, Unchecked...>, std::tuple<Checked...>>
    : rebind_sequence_container_helper<
          Container, Rebind, std::tuple<Unchecked...>,
          std::tuple<Checked..., typename rebind_alloc_if_matches<
                                     Head, typename Container::allocator_type,
                                     Rebind>::type>> {};

// Utility function to determine whether Container is an inner struct
// 'other'. If so, we will prefer to use that
template <typename Container, typename Rebind>
constexpr auto has_other(int)
    -> decltype(typename Container::template other<Rebind>{}, bool{}) {
    return true;
}

template <typename Container, typename Rebind>
constexpr bool has_other(long) {
    return false;
}

template <template <typename...> class Container, typename Rebind,
          bool has_other, typename... Args>
struct rebind_prefer_other;

// If 'Container' does not have an other, use the logic above to attempt
// to rebind it.
template <template <typename...> class Container, typename Rebind,
          typename... Args>
struct rebind_prefer_other<Container, Rebind, false, Args...> {
    using other = typename rebind_sequence_container_helper<
        Container<Args...>, Rebind, std::tuple<Args...>, std::tuple<>>::type;
};

// If 'Container' does have an 'other', use it to rebind.
template <template <typename...> class Container, typename Rebind,
          typename... Args>
struct rebind_prefer_other<Container, Rebind, true, Args...> {
    using other = typename Container<Args...>::template other<Rebind>;
};

// If 'Container' is not a template class, it must define 'other'
template <typename Container>
struct rebind_sequence_container {
    template <typename Rebind>
    using other = typename Container::template other<Rebind>;
};

// Rebind 'Container' using 'other', if it exists, and if not,
// by changing the first template parameter and rebinding any
// template parameters of Container::allocator_type
template <template <typename...> class Container, typename... Args>
struct rebind_sequence_container<Container<Args...>> {
    template <typename Rebind>
    using other =
        typename rebind_prefer_other<Container, Rebind,
                                     has_other<Container<Args...>, Rebind>(0),
                                     Args...>::other;
};

// Determine whether \a Container is random access by examining its iterator
// traits
template <typename Container>
using is_random_access =
    typename std::is_same<typename std::iterator_traits<
                              typename Container::iterator>::iterator_category,
                          std::random_access_iterator_tag>;

// Given a pack of containers, type is a tuple of their value_types
template <typename... Containers>
struct pack_value_types {
    using type =
        decltype(std::make_tuple(typename Containers::value_type{}...));
};

template <std::size_t Num, std::size_t... Nums>
struct index_pack : index_pack<Num - 1, Num, Nums...> {};

template <std::size_t... Nums>
struct index_pack<0, Nums...> {
    using type = index_pack<0, Nums...>;
};

template <std::size_t UpperBound>
constexpr auto make_index_pack() -> typename index_pack<UpperBound - 1>::type {
    return {};
}

// Given a tuple of iterators, return a tuple of the values of each iterator
template <typename... Ts>
auto get_iter_tuple_value(const std::tuple<Ts...>& tuple)
    -> decltype(get_iter_tuple_value_helper(tuple,
                                            make_index_pack<sizeof...(Ts)>())) {
    return get_iter_tuple_value_helper(tuple, make_index_pack<sizeof...(Ts)>());
}

template <typename Tuple, std::size_t... Indexes>
auto get_iter_tuple_value_helper(const Tuple& tuple, index_pack<Indexes...>)
    -> decltype(std::make_tuple(*std::get<Indexes>(tuple)...)) {
    return std::make_tuple(*std::get<Indexes>(tuple)...);
}

// C++11 does not allow expansions of variadic packs across a function
// outside of a function call, so just expand into an empty function
template <typename... T>
void allow_naked_pack_expansion(T&&...) {}

// Advance each iterator in a tuple of iterators by 1
template <typename... Ts>
void advance_iter_tuple(std::tuple<Ts...>& tuple) {
    advance_iter_tuple_helper(tuple, make_index_pack<sizeof...(Ts)>());
}

template <typename Tuple, std::size_t... Indexes>
void advance_iter_tuple_helper(Tuple& tuple, index_pack<Indexes...>) {
    allow_naked_pack_expansion(
        (std::advance(std::get<Indexes>(tuple), 1), 0)...);
}

// Some sequence containers (like forward_list) do not have a size()
// member function, so invoke std::distance to determine their size
template <typename Container>
auto container_size_helper(const Container& c, long)
    -> decltype(std::distance(c.begin(), c.end())) {
    return std::distance(c.begin(), c.end());
}

template <typename Container>
auto container_size_helper(const Container& c, int) -> decltype(c.size()) {
    return c.size();
}

template <typename Container>
auto container_size(const Container& c)
    -> decltype(container_size_helper(c, 0)) {
    return container_size_helper(c, 0);
}

} // end detail
} // end cec

#endif
