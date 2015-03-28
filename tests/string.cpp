#include <gtest/gtest.h>
#include <cec/string.hpp>
#include <cec/forward_list.hpp>

TEST(string, constructor) {
    cec::string str1;
    cec::string str2 = "from char array";
    cec::string str3(10, 'A');
    cec::string str4 = {'a', 'b', 'c', 'd'};

    EXPECT_TRUE(str1.empty());
    EXPECT_EQ(str2, "from char array");
    EXPECT_EQ(str3, "AAAAAAAAAA");
    EXPECT_EQ(str4, "abcd");
}

TEST(string, split) {
    cec::string msg = "hello world";
    cec::vector<cec::string> split = msg.split();
    cec::vector<cec::string> compare = {"hello", "world"};
    EXPECT_EQ(split, compare);

    msg = "A longer message, with more words";
    split = msg.split();
    compare = {"A", "longer", "message,", "with", "more", "words"};
    EXPECT_EQ(split, compare);

    msg = "word";
    split = msg.split();
    compare = {msg};
    EXPECT_EQ(split, compare);
}

TEST(string, join) {
    cec::forward_list<cec::string> parts = {"hello", "world"};
    cec::string joined = cec::string(", ").join(parts);
    EXPECT_EQ(joined, "hello, world");

    parts = {"word"};
    joined = cec::string(", ").join(parts);
    EXPECT_EQ(joined, "word");
}

TEST(string, to_lower) {
    cec::string msg = "A mixed Case MeSSaGe.";
    EXPECT_EQ(msg.to_lower(), "a mixed case message.");
}

TEST(string, to_upper) {
    cec::string msg = "A mixed Case MeSSaGe.";
    EXPECT_EQ(msg.to_upper(), "A MIXED CASE MESSAGE.");
}
