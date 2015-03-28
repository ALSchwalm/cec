#ifndef CEC_EXTENDED_SEQUENCE_CONTAINER
#define CEC_EXTENDED_SEQUENCE_CONTAINER

#include <algorithm>
#include <utility>
#include <type_traits>
#include <iterator>
#include <cec/detail/extended_sequence_container.hpp>

/**
 * The cec namespace contains mixins for the various container types
 * which extend them with convenient functionality.
 */
namespace cec {

/**
 * @brief The extended container mixin for types satisfying the requirements
 * for <a href="http://en.cppreference.com/w/cpp/concept/SequenceContainer">
 * SequenceContainer</a>.
 *
 * As some functions provided by the extended sequence container interface
 * can produce containers of a different type (e.g., map(), zip(), flatten()),
 * This new type is produced by 'rebinding' the container using the following
 * rules:
 *
 * - If \a SequenceContainer is not a template class, it must provide a
 * template alias `other`, which yields an appropriate sequence container type
 * for storing an arbitrary type 'T' (i.e, <code> typename
 * SequenceContainer::template other<int> </code> must be a sequence container
 * capable of holding int).
 *
 * - Otherwise, if \a SequenceContainer is a template class, and provides
 * a template alias as described above, that alias will be used to determine
 * the appropriate container type.
 *
 * - Otherwise, the first template parameter of \a SequenceContainer will be
 * replace with 'T', and any template parameter matching
 * SequenceContainer::allocator_type will be rebound using
 * std::allocator_traits.
 */
template <typename SequenceContainer>
class extended_sequence_container : public SequenceContainer {

    // Convience alias. Rebinds a sequence container to hold some type T,
    // and wraps that type in an extended_sequence_container
    template <typename T>
    using rebind_as_extended_container =
        extended_sequence_container<typename detail::rebind_sequence_container<
            SequenceContainer>::template other<T>>;

    template <typename PairType>
    using unzip_t =
        std::pair<rebind_as_extended_container<typename PairType::first_type>,
                  rebind_as_extended_container<typename PairType::second_type>>;

    template <typename Container>
    using zip_t = rebind_as_extended_container<
        std::pair<typename SequenceContainer::value_type,
                  typename Container::value_type>>;

    template <typename... Containers>
    using zip_n_t =
        rebind_as_extended_container<typename detail::pack_value_types<
            SequenceContainer, Containers...>::type>;

public:
    // Inherit T's constructors
    using SequenceContainer::SequenceContainer;
    using typename SequenceContainer::value_type;

    /// The sequence container type being extended
    using container_type = SequenceContainer;

    /**
     * @brief Perfect forwarding constructor. This allows conversions from
     * \a SequenceContainer to extended_sequence_container
     */
    template <typename... Args>
    extended_sequence_container(Args&&... args)
        : SequenceContainer(std::forward<Args>(args)...) {}

    /**
     * @brief Create a copy of this container with the elements of \a container
     * appended
     *
     * @param[in] container - The container to append to this one
     * @return A copy of this container with the elements of \a container
     * appended
     */
    template <typename Container>
    extended_sequence_container concat(const Container& container) const & {
        return extended_sequence_container(*this).extend(container);
    }

    // When 'this' is a modifiable r-value, just extend in-place
    template <typename Container>
    extended_sequence_container concat(const Container& container) && {
        return std::move(extend(container));
    }

    /**
     * @brief Test whether \a value is within this container
     *
     * @param[in] value - The value to check for
     * @return \p true if \a value is within this container, \p false otherwise
     */
    template <typename T>
    bool contains(const T& value) const {
        return std::find(this->begin(), this->end(), value) != this->end();
    }

    /**
     * @brief Counts the elements equal to \a value in this container
     * @param[in] value - The value to count
     * @returns The number of occurrences of \a value
     *
     * \see To count using a predicate: count_if()
     */
    typename SequenceContainer::difference_type
    count(const value_type& value) const {
        return std::count(this->begin(), this->end(), value);
    }

    /**
     * @brief Count the occurrences of values satisfying a predicate in
     * this container
     *
     * @param[in] p - The predicate
     * @returns The number of values which satisfy \a p
     *
     * \see To count occurences of a value: count()
     */
    template <typename UnaryPredicate>
    typename SequenceContainer::difference_type
    count_if(UnaryPredicate p) const {
        return std::count_if(this->begin(), this->end(), p);
    }

    /**
     * @brief Remove all items in this container that are equal to value
     * @param[in] value - The value to compare each item against
     * @returns A reference to this container
     */
    extended_sequence_container erase_all(const value_type& value) {
        this->erase(std::remove(this->begin(), this->end(), value),
                    this->end());
        return *this;
    }

    /**
     * @brief Modify this container such that items failing to satisfy a
     * predicate are removed.
     *
     * @param[in] p - The predication function. Elements where \a p(element)
     *                returns \a true will be removed
     * @return A reference to this container
     *
     * @note The effect of the predicate returning \a true is the reverse of
     *       filter(). This is simply to preserve the conventional definition
     *       of these functions.
     *
     * \see The non-modifying version: filter()
     */
    template <typename UnaryPredicate>
    extended_sequence_container& erase_if(UnaryPredicate p) {
        this->erase(std::remove_if(this->begin(), this->end(), p), this->end());
        return *this;
    }

    /**
     * @brief Append the contents of \a container to the end of this container
     *
     * @param[in] container - The container to append to this one
     * @return A reference to this container
     *
     * \see The non-modifying version: concat()
     */
    template <typename Container>
    extended_sequence_container& extend(const Container& container) {
        this->insert(this->end(), container.begin(), container.end());
        return *this;
    }

    /**
     * @brief Create a copy of this container with all elements that satisfy
     *        the predicate function.
     *
     * @param[in] p - The predication function. Elements where \a p(element)
     *                returns \a false will be removed
     * @return The filtered container
     *
     * \see The modifying version: erase_if
     */
    template <typename UnaryPredicate>
    extended_sequence_container filter(UnaryPredicate p) const & {
        extended_sequence_container temp;
        for (const auto& item : *this) {
            if (p(item)) {
                temp.emplace(temp.end(), item);
            }
        }
        return temp;
    }

    // If 'this' is a modifiable r-value, just filter in-place.
    template <typename UnaryPredicate>
    extended_sequence_container filter(UnaryPredicate p) && {
        return std::move(
            this->erase_if([&p](const value_type& v) { return !p(v); }));
    }

    /**
     * @brief Convert a container of containers into a single container
     * @return A copy of this container with one level of nesting removed
     */
    template <typename Container = value_type>
    Container flatten() const & {
        Container flattened;

        for (const auto& innerContainer : *this) {
            flattened.insert(flattened.end(), innerContainer.begin(),
                             innerContainer.end());
        }

        return flattened;
    }

    // If 'this' is a modifiable r-value, we can destroy the inner containers
    // while we build the new one
    template <typename Container = value_type>
    Container flatten() && {
        Container flattened;

        for (auto&& innerContainer : *this) {
            flattened.insert(flattened.end(),
                             std::make_move_iterator(innerContainer.begin()),
                             std::make_move_iterator(innerContainer.end()));
        }

        return flattened;
    }

    /**
     * @brief Create a container by applying a function to each element of
     * this container
     *
     * The type of the returned container will be an extended container of
     * \a T, but with the return type of \a f in place of the first template
     * parameter.
     *
     * @param[in] f - The function to map across this container
     * @return A new container with \a f applied to each element
     *
     * \see The in-place version: transform()
     */
    template <typename UnaryFunction>
    auto map(UnaryFunction f) const
        & -> rebind_as_extended_container<decltype(f(*this->begin()))> {

        extended_sequence_container<typename detail::rebind_sequence_container<
            SequenceContainer>::template other<decltype(f(*this->begin()))>>
            mapped;
        for (const auto& item : *this) {
            mapped.emplace(mapped.end(), f(item));
        }
        return mapped;
    }

    // In the special case that 'this' is an r-value and f maps value_type ->
    // value_type, the operation can be performed in-place.
    template <typename UnaryFunction>
        auto map(UnaryFunction f) &&
        -> typename std::enable_if<
            std::is_same<decltype(f(*this->begin())), value_type>::value,
            extended_sequence_container>::type {
        return std::move(this->transform(f));
    }

    /**
     * @brief Reduces the elements of this container using the associative
     * function \a f.
     *
     * @param[in] f - Associative function to use to reduce contents
     * @param[in] init - The initial element of the reduction
     * @return The value of the reduction
     */
    template <typename BinaryFunction, typename Init>
    Init reduce(BinaryFunction f, Init init) const & {
        return std::accumulate(this->begin(), this->end(), init, f);
    }

    // If 'this' is modafiable, we can move from the underlying container
    template <typename BinaryFunction, typename Init>
    Init reduce(BinaryFunction f, Init init) && {
        return std::accumulate(std::make_move_iterator(this->begin()),
                               std::make_move_iterator(this->end()), init, f);
    }

    /**
     * @brief Reduces the elements of this container using the associative
     * function \a f.
     *
     * The initial element of the reduction is the first element of this
     * container.
     *
     * @param[in] f - Associative function to use to reduce contents
     * @return The value of the reduction
     *
     * Example Usage:
     * @code
     *    cec::vector<std::string> msg_parts = {"Hel", "lo", ", wo", "rld"};
     *    std::string msg = msg_parts.reduce([](const std::string& msg,
     *                                          const std::string& part){
     *        return msg + part;
     *    });
     *    // msg == "Hello, world"
     * @endcode
     */
    template <typename BinaryFunction>
    value_type reduce(BinaryFunction f) const {
        return std::accumulate(std::next(this->begin()), this->end(),
                               *this->begin(), f);
    }

    /**
     * @brief Sort this container with the given comparator.
     *
     * If \a T provides a RandomAccessIterator, then \a std::sort will be
     * called. Otherwise, \a T must provide a member function named "sort",
     * which will be invoked.
     *
     * @param[in] comp - The comparator to use while sorting
     * @return A reference to this container (now sorted)
     */
    template <typename Compare = std::less<value_type>>
    extended_sequence_container& sort(Compare comp = Compare{}) {
        sort_helper(comp, typename detail::is_random_access<
                              SequenceContainer>::type{});
        return *this;
    }

    /**
     * @brief Create a new seqence from the initial elements of this sequence
     *
     * @param[in] num - The number of elements to take from this sequence
     * @return A new sequence from the taken elements
     */
    extended_sequence_container
    take(typename SequenceContainer::difference_type num) const & {
        return {this->begin(), std::next(this->begin(), num)};
    }

    // When 'this' is a modifiable r-value, just erase in-place
    extended_sequence_container
    take(typename SequenceContainer::difference_type num) && {
        this->erase(std::next(this->begin(), num), this->end());
        return std::move(*this);
    }

    /**
     * @brief Create a new container from the initial elements of this container
     * satisfying a predicate
     *
     * @param[in] p - The predicate being satisfied
     * @return A new sequence from the taken elements
     */
    template <typename UnaryPredicate>
    extended_sequence_container take_while(UnaryPredicate p) const & {
        auto end = std::find_if_not(this->begin(), this->end(), p);
        return {this->begin(), end};
    }

    // When 'this' is a modifiable r-value, erase in-place
    template <typename UnaryPredicate>
    extended_sequence_container take_while(UnaryPredicate p) && {
        auto last = std::find_if_not(this->begin(), this->end(), p);
        this->erase(last, this->end());
        return std::move(*this);
    }

    /**
     * @brief Convert this container to some other type
     *
     * Conversion is performed as though by calling \a Container(begin(), end())
     *
     * @return The converted container
     */
    template <typename Container>
    Container to() {
        return Container(this->begin(), this->end());
    }

    /**
     * @brief Apply a function to each element of this container and store the
     * result in-place.
     *
     * @param[in] f - The function to use to transform each element
     * @return A reference to this container (now modified)
     *
     * \see The non-modifying version: map()
     */
    template <typename UnaryFunction>
    extended_sequence_container& transform(UnaryFunction f) {
        std::transform(std::make_move_iterator(this->begin()),
                       std::make_move_iterator(this->end()), this->begin(), f);
        return *this;
    }

    /**
     * @brief Transform this sequence of pairs in to a pair of sequences
     * @return A pair of sequences
     *
     * Example Usage:
     * @code
     *    cec::vector<std::pair<int, float>> vec_of_pairs = {
     *        {1, 5.0},
     *        {2, 17.2}
     *        {3, 98.6}
     *    };
     *
     *    // vector containing { {1, 2, 3}, {5.0, 17.2, 98.6} }
     *    auto pair_of_vecs = vec_of_pairs.unzip();
     * @endcode
     */
    template <typename PairType = value_type>
    unzip_t<PairType> unzip() const {
        unzip_t<PairType> unzipped;

        for (const auto& item : *this) {
            unzipped.first.emplace(unzipped.first.end(), item.first);
            unzipped.second.emplace(unzipped.second.end(), item.second);
        }

        return unzipped;
    }

    /**
     * @brief Create a sequence of the element-wise pairing of the container and
     * \a other
     *
     * @return A sequence of pairs
     *
     * Example Usage:
     * @code
     *    cec::vector<int> left = {1, 2, 3};
     *    cec::vector<char> right = {'a', 'b', 'c'};
     *
     *    // contains { {1, 'a'}, {2, 'b'}, {3, 'c'} }
     *    cec::vector<std::pair<int, char>> zipped = left.zip(right);
     * @endcode
     */
    template <typename Container>
    zip_t<Container> zip(const Container& other) const {
        zip_t<Container> zipped;

        auto first_iter = this->begin();
        auto second_iter = other.begin();

        for (; first_iter != this->end() && second_iter != other.end();
             std::advance(first_iter, 1), std::advance(second_iter, 1)) {
            zipped.emplace(zipped.end(), *first_iter, *second_iter);
        }

        return zipped;
    }

    /**
     * @brief Create a sequence of tuples from this container and the provided
     * \a containers.
     *
     * Like \a zip but with an arbitrary number of containers. The return type
     * \a zip_n_t will be an extended container of \a T with the first parameter
     * rebound to a tuple of the value_type of this container and each container
     * in \a containers.
     *
     * @return A sequence of tuples
     */
    template <typename... Containers>
    zip_n_t<Containers...> zip_n(const Containers&... containers) const {
        zip_n_t<Containers...> zipped;

        // FIXME this is just generally bad
        std::array<std::size_t, sizeof...(Containers) + 1> sizes =
            {detail::container_size(*this),
             detail::container_size(containers)...};

        auto smallest = *std::min_element(sizes.begin(), sizes.end());
        auto iter_tuple =
            std::make_tuple(this->begin(), std::begin(containers)...);

        for (std::size_t i = 0; i < smallest; ++i) {
            zipped.emplace(zipped.end(),
                           detail::get_iter_tuple_value(iter_tuple));
            detail::advance_iter_tuple(iter_tuple);
        }

        return zipped;
    }

private:
    template <typename Compare>
    void sort_helper(Compare comp, std::true_type) {
        std::sort(this->begin(), this->end(), comp);
    }

    template <typename Compare>
    void sort_helper(Compare comp, std::false_type) {
        SequenceContainer::sort(comp);
    }
};
} // end cec

#endif
