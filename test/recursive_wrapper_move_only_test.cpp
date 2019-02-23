/*=============================================================================
  Copyright (c) 2019 Nikita Kniazev

  Distributed under the Boost Software License, Version 1.0. (See accompanying
  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// Check that move-only types can be used with recursive_wrapper

#include <boost/config.hpp>
#if defined(BOOST_NO_CXX11_RVALUE_REFERENCES) || defined(BOOST_NO_CXX11_NOEXCEPT)
int main() {}
#else
#include "boost/core/lightweight_test.hpp"
#include <boost/variant.hpp>
#include <boost/type_traits/is_nothrow_move_constructible.hpp>
#include <boost/type_traits/is_nothrow_move_assignable.hpp>
#include <boost/type_traits/has_nothrow_constructor.hpp>
#include <boost/type_traits/has_nothrow_assign.hpp>
#include <utility>

// pointer stealing does not care if move can throw
struct move_only_type
{
    explicit move_only_type(int value) : value_(value) {}
    move_only_type(move_only_type&& x) : value_(x.value_) {}
    move_only_type& operator=(move_only_type&& x) { value_ = x.value_; return *this; }

    int value_;
};
static_assert(!boost::is_nothrow_move_constructible<move_only_type>::value, "");
#if !BOOST_WORKAROUND(BOOST_GCC_VERSION, < 40700)
static_assert(!boost::is_nothrow_move_assignable<move_only_type>::value, "");
#else // boostorg/type_traits#114
namespace boost {
template <> struct is_nothrow_move_assignable<move_only_type> : boost::false_type {};
}
#endif

void test_noexcept()
{
    boost::variant<
        boost::recursive_wrapper<int>,
        boost::recursive_wrapper<move_only_type>
    > a(move_only_type(0)), b(move_only_type(1)), c(3);
    BOOST_TEST(!a.empty());
    BOOST_TEST(!b.empty());
    a = std::move(b);
    BOOST_TEST(!a.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(a).value_, 1);
    // check that we can reuse a moved out variant with move assignment
    move_only_type c_(2);
    b = std::move(c_); // TODO: no emplace yet
    BOOST_TEST(!b.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(b).value_, 2);
    // check that recursive variant advertises its valueless after beging moved out
    BOOST_TEST_EQ(boost::get<int>(c), 3);
    c = std::move(b);
    BOOST_TEST( b.empty());
    BOOST_TEST_EQ(boost::get<move_only_type>(c).value_, 2);
}

struct throwing_type
{
    throwing_type() {}
    throwing_type(throwing_type const&) { throw 0l; }
    throwing_type& operator=(throwing_type const&) { throw 0l; }
};
static_assert(!boost::has_nothrow_default_constructor<throwing_type>::value, "");
static_assert(!boost::has_nothrow_copy_constructor<throwing_type>::value, "");
//static_assert(!std::is_nothrow_copy_assignable<throwing_type>::value, ""); // no boost alternative

void test_throwing()
{
    boost::variant<
        throwing_type,
        boost::recursive_wrapper<int>
    > a, b(1);

    BOOST_TEST_THROWS(b = std::move(a), long);
    // failed move must not change anything
    BOOST_TEST(!a.empty());
    BOOST_TEST(!b.empty());
    BOOST_TEST_EQ(a.which(), 0);
    BOOST_TEST_EQ(boost::get<int>(b), 1);
}

int main()
{
    test_noexcept();
    test_throwing();
    return boost::report_errors();
}
#endif
