/*=============================================================================
  Copyright (c) 2019 Nikita Kniazev

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// Check that move-only types can be used with recursive_wrapper

#include <boost/config.hpp>
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
// variant does not support move emulation
int main() {}
#else
#include "boost/core/lightweight_test.hpp"
#include <boost/variant.hpp>
#include <boost/move/utility_core.hpp>

struct move_only_type
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(move_only_type)

public:
    explicit move_only_type(int value) BOOST_NOEXCEPT
      : value_(value)
    {}

    // Move ctor
    move_only_type(BOOST_RV_REF(move_only_type) x) BOOST_NOEXCEPT
      : value_(x.value_)
    {}

    // Move assign
    move_only_type& operator=(BOOST_RV_REF(move_only_type) x) BOOST_NOEXCEPT
    {
        value_ = x.value_;
        return *this;
    }

    int value_;
};

int main()
{
    // TODO: variant does not have inplace construction yet
    move_only_type a_(0), b_(1);
    boost::variant<
        boost::recursive_wrapper<int>,
        boost::recursive_wrapper<move_only_type>
    > a(boost::move(a_)), b(boost::move(b_)), c(3);
    BOOST_TEST(!a.empty());
    BOOST_TEST(!b.empty());
    a = boost::move(b);
    BOOST_TEST(!a.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(a).value_, 1);
    // check that we can reuse a moved out variant with move assignment
    move_only_type c_(2);
    b = boost::move(c_); // TODO: no emplace yet
    BOOST_TEST(!b.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(b).value_, 2);
    // check that recursive variant advertises its valueless after beging moved out
    BOOST_TEST_EQ(boost::get<int>(c), 3);
    c = boost::move(b);
    BOOST_TEST( b.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(c).value_, 2);
    return boost::report_errors();
}
#endif
