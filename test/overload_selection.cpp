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

// Pre msvc-14.0 could not dustinguish between multiple assignment operators:
//      warning C4522: 'assignment_tester' : multiple assignment operators specified
//      error C2666: variant::operator =' : 3 overloads have similar conversions
// Old versions of GCC have same issue:
//      error: variant::operator=(const T&) cannot be overloaded
#if (defined(__GNUC__) && (__GNUC__ < 4)) || (defined(_MSC_VER) && _MSC_VER < 1900)

void test_overload_selection_variant_assignment() {
    BOOST_CHECK(true);
}

#else

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

#endif

typedef boost::variant<int> my_variant;

struct convertible {
    operator my_variant() const {
        return my_variant();
    }
};

void test_implicit_conversion_operator() {
    // https://svn.boost.org/trac/boost/ticket/8555
    my_variant y = convertible();
    BOOST_CHECK(y.which() == 0);
}

struct X: boost::variant< int > {};
class V1: public boost::variant<float,double> {};

struct AB: boost::variant<A, B> {};

void test_derived_from_variant_construction() {
    // https://svn.boost.org/trac/boost/ticket/7120
    X x;
    boost::variant<X> y(x);
    BOOST_CHECK(y.which() == 0);

    // https://svn.boost.org/trac/boost/ticket/10278
    boost::variant<V1, std::string> v2 = V1();
    BOOST_CHECK(v2.which() == 0);

    // https://svn.boost.org/trac/boost/ticket/12155
    AB ab;
    boost::variant<AB, C> ab_c(ab); // member function convert_construct is abiguous
    BOOST_CHECK(ab_c.which() == 0);

    boost::variant<A, B> a_b(ab);
    BOOST_CHECK(a_b.which() == 0);

    boost::variant<B, C, A> b_c_a1(static_cast<boost::variant<A, B>& >(ab));
    BOOST_CHECK(b_c_a1.which() == 2);


//  Following conversion seems harmful as it may lead to slicing:
//  boost::variant<B, C, A> b_c_a(ab);
//  BOOST_CHECK(b_c_a.which() == 2);
}

int test_main(int , char* [])
{
    test_overload_selection_variant_constructor();
    test_overload_selection_variant_assignment();
    test_implicit_conversion_operator();
    test_derived_from_variant_construction();

    return boost::exit_success;
}
