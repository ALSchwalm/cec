#include <gtest/gtest.h>
#include <cec/extended_sequence_container.hpp>
#include <cec/deque.hpp>
#include <cec/forward_list.hpp>
#include <cec/list.hpp>
#include <cec/string.hpp>
#include <cec/vector.hpp>
#include <algorithm>

// To test the container rebinding behavior, we need to define a few
// structs here
namespace rebind_setup {

// MyList is not a template, rebind should just produce std::lists
struct MyList : std::list<int> {
    template <typename Rebind>
    using other = std::list<Rebind>;
};

// It should still be extendable
using extended_MyList = cec::extended_sequence_container<MyList>;

// MyTemplateList is a template type, but using 'other' should be prefered,
// to substituting rebind for T. So, std::vector should be produced when
// rebinding
template <typename T>
struct MyTemplateList : std::list<T> {
    template <typename Rebind>
    using other = std::vector<Rebind>;
};

template <typename T>
using extended_MyTemplateList =
    cec::extended_sequence_container<MyTemplateList<T>>;

} // End rebind_setup

TEST(SequenceContainer, Rebind) {
    rebind_setup::extended_MyList list{};

    // Mapping a function returning double should produce a std::list<double>
    std::list<double> f = list.map([](int i) { return 0.0; });

    rebind_setup::extended_MyTemplateList<int> t_list{};

    // Mapping a function returning double should produce a std::vector<double>
    std::vector<double> f2 = t_list.map([](int i) { return 0.0; });
}

TEST(SequenceContainer, construction) {
    cec::deque<char> deque;
    cec::forward_list<double> flist;
    cec::list<int> list;
    cec::vector<float> vec;
}

TEST(SequenceContainer, concat) {
    const cec::vector<short> numbers = {1, 2, 3};
    const cec::vector<short> other_numbers = {4, 5, 6};
    const cec::vector<short> all_numbers = {1, 2, 3, 4, 5, 6};
    EXPECT_EQ(numbers.concat(other_numbers), all_numbers);
}

TEST(SequenceContainer, contains) {
    const cec::vector<int> numbers = {1, 2, 3};
    EXPECT_TRUE(numbers.contains(1));
    EXPECT_FALSE(numbers.contains(4));
}

TEST(SequenceContainer, extend) {
    cec::vector<char> letters = {'a', 'b', 'c'};
    const cec::vector<char> other_letters = {'d', 'e', 'f'};
    const cec::vector<char> all_letters = {'a', 'b', 'c', 'd', 'e', 'f'};
    letters.extend(other_letters);
    EXPECT_EQ(letters, all_letters);
}

TEST(SequenceContainer, filter) {
    const cec::deque<int> numbers = {1, 2, 3, 4};
    cec::deque<int> filtered = numbers.filter([](int i) { return i % 2 == 0; });
    cec::deque<int> evens = {2, 4};
    EXPECT_EQ(filtered, evens);

    auto copy = numbers;
    auto out = std::move(copy).filter([](int i) { return i % 2 == 0; });
    EXPECT_EQ(out, filtered);
}

TEST(SequenceContainer, flatten) {
    cec::list<cec::vector<int>> nested = {{1, 2, 3}, {4, 5, 6}};
    cec::vector<int> flattened = nested.flatten();
    cec::vector<int> compare = {1, 2, 3, 4, 5, 6};
    EXPECT_EQ(flattened, compare);

    std::list<int> v_flattened = nested.flatten<std::list<int>>();
    std::list<int> v_compare = {1, 2, 3, 4, 5, 6};

    EXPECT_EQ(v_flattened, v_compare);
}

TEST(SequenceContainer, map) {
    const cec::deque<std::string> container = {"Some", "test", "strings"};

    cec::deque<int> mapped =
        container.map([](const std::string& s) { return s == "test" ? 1 : 0; });

    cec::deque<int> compare = {0, 1, 0};
    EXPECT_EQ(mapped, compare);
}

TEST(SequenceContainer, reduce) {
    const cec::vector<std::string> msg_parts = {"Hel", "lo", ", wo", "rld"};

    // without initial element
    std::string msg =
        msg_parts.reduce([](const std::string& msg, const std::string& part) {
            return msg + part;
        });
    EXPECT_EQ(msg, "Hello, world");

    // with initial element
    std::size_t msg_length =
        msg_parts
            .reduce([](std::size_t running_total, const std::string& part) {
                return running_total + part.size();
            }, 0);
    EXPECT_EQ(msg_length, msg.size());
}

TEST(SequenceContainer, sort) {
    // Container with random access iterator
    cec::vector<int> numbers = {3, 2, 1, 15, 2, 15};
    numbers.sort();
    EXPECT_TRUE(std::is_sorted(numbers.begin(), numbers.end()));

    // Container with member sort function
    cec::list<char> letters = {'a', 'r', 't', 'u', 'z'};
    letters.sort();
    EXPECT_TRUE(std::is_sorted(letters.begin(), letters.end()));
}

TEST(SequenceContainer, take) {
    const cec::vector<char> f = {'a', 'b', 'c', 'd'};
    auto taken = f.take(2);
    const cec::vector<char> check = {'a', 'b'};
    EXPECT_EQ(taken, check);

    auto rvalue_test = f.map([](char c) { return 1; }).take(2);
    const cec::vector<int> check2 = {1, 1};
    EXPECT_EQ(rvalue_test, check2);
}

TEST(SequenceContainer, take_while) {
    const cec::vector<int> f = {2, 4, 6, 9, 10};
    auto taken = f.take_while([](int i) { return i % 2 == 0; });
    const cec::vector<int> check = {2, 4, 6};
    EXPECT_EQ(taken, check);

    auto rvalue_test =
        f.map([](int c) { return 1; }).take_while([](int i) { return i == 1; });
    const cec::vector<int> check2 = {1, 1, 1, 1, 1};
    EXPECT_EQ(rvalue_test, check2);
}

TEST(SequenceContainer, transform) {
    cec::vector<short> container = {1, 2, 3, 4, 5};
    container.transform([](short s) { return 1 << s; });
    cec::vector<short> compare = {2, 4, 8, 16, 32};

    EXPECT_EQ(container, compare);
}

TEST(SequenceContainer, unzip) {
    cec::vector<std::pair<char, std::string>> c = {{'a', "apple"},
                                                   {'b', "bear"},
                                                   {'c', "cat"}};

    std::pair<cec::vector<char>, cec::vector<std::string>> unzipped =
        {{'a', 'b', 'c'}, {"apple", "bear", "cat"}};

    EXPECT_EQ(c.unzip(), unzipped);
}

TEST(SequenceContainer, zip) {
    cec::vector<int> c = {4, 3, 2, 1};
    cec::list<float> f = {1, 2, 3, 4};
    cec::vector<std::pair<int, float>> zipped2 = c.zip(f);
}

TEST(SequenceContainer, zip_n) {
    cec::vector<int> c = {4, 3, 2, 1};
    cec::list<short> f = {1, 2, 3, 4};
    cec::string s = "cats";
    cec::vector<std::tuple<int, short, char>> zipped3 = c.zip_n(f, s);

    cec::vector<std::tuple<int, short, char>> compare =
        {std::make_tuple(4, 1, 'c'), std::make_tuple(3, 2, 'a'),
         std::make_tuple(2, 3, 't'), std::make_tuple(1, 4, 's')};
    EXPECT_EQ(zipped3, compare);
}
