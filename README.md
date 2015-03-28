C++ Extended Container Library
==============================

Introduction
------------

The C++ Extended Container Library (CEC) is an effort to provide minins for
various C++ container types which add useful functions. Below is a short example
of the usage of CEC.

    #include <cec/vector.hpp>
    #include <cec/string.hpp>
    #include <iostream>

    using fragment_t = cec::vector<cec::vector<cec::string>>;

    int main() {
        fragment_t fragments = {{"Hel", "lo"}, {", wor", "ld"}};
        cec::string msg = cec::string{}.join(fragments.flatten());
        std::cout << msg << std::endl; // "Hello, world"
        return 0;
    }

How it works
------------

This library is largely an alternative to range libraries (e.g.,
[Boost.Range](http://www.boost.org/doc/libs/1_57_0/libs/range/doc/html/range/introduction.html)
[range-v3](https://github.com/ericniebler/range-v3)). That is not to say
that CEC is as sophisticated as these works; it is not. Generally, range libraries
are able to achieve relatively inexpensive operations like mapping and filtering,
without actually copying the underling containers. CEC, as a general rule, does
perform a copy on non-modifying operations.

The interface for CEC is inspired by the Scala standard library.
