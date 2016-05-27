//-----------------------------------------------------------------------------
// boost-libs variant/test/variant_get_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2016 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


// This test suite was created to cover issues reported in:
//      https://svn.boost.org/trac/boost/ticket/5871
//      https://svn.boost.org/trac/boost/ticket/11602

#include "boost/variant/variant.hpp"
#include "boost/test/minimal.hpp"

struct A{};
struct B{};
struct C{};
struct D{};


bool foo(const boost::variant<A, B>& ) {
    return false;
}

bool foo(const boost::variant<C, D>& ) {
    return true;
}

void test_overload_selection_variant_constructor() {
    D d;
    BOOST_CHECK(foo(d));

    boost::variant<B, A> v;
    BOOST_CHECK(!foo(v));
}


struct assignment_tester: boost::variant<C, D>, boost::variant<B, A> {
    using boost::variant<B, A>::operator=;
    using boost::variant<C, D>::operator=;
};

void test_overload_selection_variant_assignment() {
    A a;
    assignment_tester tester;
    tester = a;
    const int which0 = static_cast< boost::variant<B, A>& >(tester).which();
    BOOST_CHECK(which0 == 1);

    boost::variant<A, B> b;
    b = B();
    tester = b;
    const int which1 = static_cast< boost::variant<B, A>& >(tester).which();
    BOOST_CHECK(which1 == 0);
}


typedef boost::variant<int> my_variant;

struct convertible {
    operator my_variant() const {
        return my_variant();
    }
};

void test_implicit_conversion_operator() {
    // https://svn.boost.org/trac/boost/ticket/11602
    my_variant y = convertible();
    BOOST_CHECK(y.which() == 0);
}

int test_main(int , char* [])
{
    test_overload_selection_variant_constructor();
    test_overload_selection_variant_assignment();
    test_implicit_conversion_operator();

    return boost::exit_success;
}
