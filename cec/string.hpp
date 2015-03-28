#ifndef CECL_STRING
#define CECL_STRING

#include <string>
#include <regex>
#include <cec/extended_sequence_container.hpp>
#include <cec/vector.hpp>

namespace cec {

/**
 * @brief Adaptor that allows basic_string to be used with
 * extended_sequence_container
 *
 * As the name implies, this type is not an extended_container. Instead
 * it acts as an apator allowing \a std::basic_string to be used with
 * extended_sequence_container. This adaptor also adds some utility
 * functions such as \a join and \a split.
 */
template <class CharT, class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
class extendable_basic_string
    : public std::basic_string<CharT, Traits, Allocator> {
public:
    using std::basic_string<CharT, Traits, Allocator>::basic_string;

    template <typename... Args>
    extendable_basic_string(Args&&... args)
        : std::basic_string<CharT, Traits, Allocator>(
              std::forward<Args>(args)...) {}

    /**
     * @brief Split this string on \a delimiter.
     *
     * @param[in] delimiter - The regex delimiter
     *
     * @return The split string in a container of type \a Container (by default
     * cec::vector<cec::string>)
     *
     * Example Usage:
     * @code
     *    cec::string msg = "hello world";
     *    cec::vector<cec::string> compare = {"hello", "world"};
     *    cec::vector<cec::string> split = msg.split();
     *
     *    // compare == split
     * @endcode
     */
    template <typename Container = cec::vector<
                  cec::extended_sequence_container<extendable_basic_string>>>
    Container split(std::regex delimiter = std::regex("\\S+")) const {
        Container container;
        auto iter = std::sregex_iterator(this->begin(), this->end(), delimiter);
        for (; iter != std::sregex_iterator{}; ++iter) {
            container.emplace(container.end(), iter->str());
        }
        return container;
    }

    /**
     * @brief Join a collection of strings together using this string as
     * the delimter
     *
     * @param[in] strings - The collection of strings to join together
     *
     * @return The joined strings
     *
     * Example Usage:
     * @code
     *    cec::forward_list<cec::string> parts = {"hello", "world"};
     *    cec::string joined = cec::string(", ").join(parts);
     *    // joined == "hello, world"
     * @endcode
     */
    template <typename Container>
    cec::extended_sequence_container<extendable_basic_string>
    join(const Container& strings) const {
        cec::extended_sequence_container<extendable_basic_string> joined;

        if (strings.empty()) {
            return joined;
        }

        auto iter = strings.begin();
        for (; std::next(iter) != strings.end(); ++iter) {
            joined += (*iter + *this);
        }
        joined += *iter;
        return joined;
    }

    /**
     * Create a copy of this string converted to lower case
     *
     * @note This method is only suitable for converting case of
     * ASCII encoded strings.
     *
     * @returns The lowercase string
     *
     * \see To convert to upper case: to_upper()
     */
    cec::extended_sequence_container<extendable_basic_string> to_lower() const {
        // TODO depend on boost for encoding awareness?
        auto lowered = *this;
        for (auto& letter : lowered) {
            letter = std::tolower(letter);
        }
        return lowered;
    }

    /**
     * Create a copy of this string converted to upper case
     *
     * @note This method is only suitable for converting case of
     * ASCII encoded strings.
     *
     * @returns The uppercase string
     *
     * \see To conver to lower case: to_lower()
     */
    cec::extended_sequence_container<extendable_basic_string> to_upper() const {
        // TODO depend on boost for encoding awareness?
        auto uppered = *this;
        for (auto& letter : uppered) {
            letter = std::toupper(letter);
        }
        return uppered;
    }

protected:
    template <typename Iter>
    auto emplace(Iter pos, CharT c) -> decltype(this->insert(pos, c)) {
        return this->insert(pos, c);
    }
};

/**
 * @brief The extended basic_string type
 */
template <class CharT, class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
using basic_string = cec::extended_sequence_container<
    extendable_basic_string<CharT, Traits, Allocator>>;

/**
 * @brief A convenience alias for cec::basic_string<char>
 */
using string = cec::basic_string<char>;

/**
 * @brief A convenience alias for cec::basic_string<wchar_t>
 */
using wstring = cec::basic_string<wchar_t>;

/**
 * @brief A convenience alias for cec::basic_string<char16_t>
 */
using u16string = cec::basic_string<char16_t>;

/**
 * @brief A convenience alias for cec::basic_string<char32_t>
 */
using u32string = cec::basic_string<char32_t>;
}

#endif
