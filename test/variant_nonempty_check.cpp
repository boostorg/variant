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

void prevent_compiler_noexcept_detection() {
    char* p = new char;
    *p = '\0';
    delete p;
}

struct throwing_class {
    int trash;
    enum helper_enum { do_not_throw = 777 };

    throwing_class(int value = 123) BOOST_NOEXCEPT_IF(false) : trash(value) {
        prevent_compiler_noexcept_detection();
    }

    throwing_class(const throwing_class& b) BOOST_NOEXCEPT_IF(false) : trash(b.trash) {
        if (trash != do_not_throw) {
            throw exception_on_assignment();
        }
    }

    const throwing_class& operator=(const throwing_class& b) BOOST_NOEXCEPT_IF(false) {
        trash = b.trash;
        if (trash != do_not_throw) {
            throw exception_on_assignment();
        }

        return *this;
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    throwing_class(throwing_class&& b) BOOST_NOEXCEPT_IF(false) : trash(b.trash) {
        if (trash != do_not_throw) {
            throw exception_on_move_assignment();
        }
    }

    const throwing_class& operator=(throwing_class&& b) BOOST_NOEXCEPT_IF(false) {
        trash = b.trash;
        if (trash != do_not_throw) {
            throw exception_on_move_assignment();
        }

        return *this;
    }
#endif

    virtual ~throwing_class() {}
};

struct nonthrowing_class {
    int trash;

    nonthrowing_class() BOOST_NOEXCEPT_IF(false) : trash(123) {
        prevent_compiler_noexcept_detection();
    }
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

inline void check_5()
{
    boost::variant<nonthrowing_class, throwing_class> v1, v2;
    throwing_class throw_not_now;
    throw_not_now.trash = throwing_class::do_not_throw;
    v1 = throw_not_now;
    v2 = throw_not_now;

    boost::get<throwing_class>(v1).trash = 1;
    boost::get<throwing_class>(v2).trash = 1;

    try {
        v1 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    }

    v1 = nonthrowing_class();
    v2 = nonthrowing_class();
    try {
        v1 = throwing_class();
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 0);
        BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
    }
    swap(v1, v2); // Make sure that two backup holders swap well
    v1 = v2;
}

inline void check_6()
{
    boost::variant<nonthrowing_class, throwing_class> v1, v2;
    throwing_class throw_not_now;
    throw_not_now.trash = throwing_class::do_not_throw;
    v1 = throw_not_now;
    v2 = throw_not_now;

    v1 = throw_not_now;
    v2 = throw_not_now;
    swap(v1, v2);
    boost::get<throwing_class>(v1).trash = 1;
    boost::get<throwing_class>(v2).trash = 1;

    v1 = throwing_class(throw_not_now);
    v2 = v1;

    v1 = nonthrowing_class();
    try {
        throwing_class tc;
        tc.trash = 2;
        v1 = tc;
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 0);
    }

    v2 = nonthrowing_class();
    try {
        v2 = 2;
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v2.which() == 0);
    }

    // Probably the most significant test:
    // unsuccessful swap must preserve old values of vaiant
    v1 = throw_not_now;
    boost::get<throwing_class>(v1).trash = 1;
    try {
        swap(v1, v2);
        BOOST_CHECK(false);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(v2.which() == 0);
        BOOST_CHECK(boost::get<throwing_class>(v1).trash == 1);
    }
}

int test_main(int , char* [])
{
    check_1();
    check_2();
    check_3();
    check_4();
    check_5();
    check_6();

    return boost::exit_success;
}
