//-----------------------------------------------------------------------------
// boost-libs variant/test/variant_nonempty_check.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2014 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/variant/variant.hpp"
#include "boost/variant/get.hpp"
#include "boost/test/minimal.hpp"
#include <stdexcept>

struct exception_on_assignment : std::exception {};
struct exception_on_move_assignment : exception_on_assignment {};

struct throwing_class {
    int trash;

    throwing_class() : trash(123) {}

    throwing_class(const throwing_class& b) : trash(b.trash) {
        throw exception_on_assignment();
    }

    const throwing_class& operator=(const throwing_class& b) {
        trash = b.trash;
        throw exception_on_assignment();

        return *this;
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    throwing_class(throwing_class&& b) : trash(b.trash) {
        throw exception_on_move_assignment();
    }

    const throwing_class& operator=(throwing_class&& b) {
        trash = b.trash;
        throw exception_on_move_assignment();

        return *this;
    }
#endif

    virtual ~throwing_class() {}
};

struct nonthrowing_class {
    int trash;

    nonthrowing_class() : trash(123) {}
};

inline void check_1()
{

    boost::variant<throwing_class, nonthrowing_class> v;
    try {
        v = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    }

    try {
        throwing_class tc;
        v = tc;
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    }
}

inline void check_2()
{

    boost::variant<nonthrowing_class, throwing_class> v;
    try {
        v = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v));
    }

    try {
        throwing_class cl;
        v = cl;
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v));
    }
}

inline void check_3()
{

    boost::variant<nonthrowing_class, throwing_class> v1, v2;

    swap(v1, v2);
    try {
        v1 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
    }


    try {
        v2 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v2));
    }

    swap(v1, v2); // Make sure that two backup holders swap well
    BOOST_CHECK(!v1.which());
    BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
    BOOST_CHECK(!v2.which());
    BOOST_CHECK(boost::get<nonthrowing_class>(&v2));

    v1 = v2;
}

inline void check_4()
{
    // This one has a fallback
    boost::variant<int, throwing_class> v1, v2;

    swap(v1, v2);
    try {
        v1 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<int>(&v1));
    }


    try {
        v2 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<int>(&v2));
    }

    swap(v1, v2);
    BOOST_CHECK(!v1.which());
    BOOST_CHECK(boost::get<int>(&v1));
    BOOST_CHECK(!v2.which());
    BOOST_CHECK(boost::get<int>(&v2));

    v1 = v2;
}

int test_main(int , char* [])
{
    check_1();
    check_2();
    check_3();
    check_4();

    return boost::exit_success;
}
