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


// In this file we are making tests to ensure that variant guarantees nonemptiness.
//
// For that purpose we create a `throwing_class`, that throws exception at a specified
// assignment attempt. If exception was thrown during move/assignemnt operation we make sure
// that data in variant is same as before move/assignemnt operation or that a fallback type is
// stored in variant.


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
    enum helper_enum {
        do_not_throw = 780,
        throw_after_5,
        throw_after_4,
        throw_after_3,
        throw_after_2,
        throw_after_1
    };

    bool is_throw() {
        if (trash < do_not_throw) {
            return true;
        }

        if (trash > do_not_throw && trash <= throw_after_1) {
            ++ trash;
            return false;
        }

        return trash != do_not_throw;
    }

    throwing_class(int value = 123) BOOST_NOEXCEPT_IF(false) : trash(value) {
        prevent_compiler_noexcept_detection();
    }

    throwing_class(const throwing_class& b) BOOST_NOEXCEPT_IF(false) : trash(b.trash) {
        if (is_throw()) {
            throw exception_on_assignment();
        }
    }

    const throwing_class& operator=(const throwing_class& b) BOOST_NOEXCEPT_IF(false) {
        trash = b.trash;
        if (is_throw()) {
            throw exception_on_assignment();
        }

        return *this;
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    throwing_class(throwing_class&& b) BOOST_NOEXCEPT_IF(false) : trash(b.trash) {
        if (is_throw()) {
            throw exception_on_move_assignment();
        }
    }

    const throwing_class& operator=(throwing_class&& b) BOOST_NOEXCEPT_IF(false) {
        trash = b.trash;
        if (is_throw()) {
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

    nonthrowing_class(const nonthrowing_class&) BOOST_NOEXCEPT_IF(false) {
        prevent_compiler_noexcept_detection();
    }

    const nonthrowing_class& operator=(const nonthrowing_class&) BOOST_NOEXCEPT_IF(false) {
        prevent_compiler_noexcept_detection();
        return *this;
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    nonthrowing_class(nonthrowing_class&&) BOOST_NOEXCEPT_IF(false) {
        prevent_compiler_noexcept_detection();
    }

    const nonthrowing_class& operator=(nonthrowing_class&&) BOOST_NOEXCEPT_IF(false) {
        prevent_compiler_noexcept_detection();
        return *this;
    }
#endif
};

inline void check_1(int helper = 1)
{
    boost::variant<throwing_class, nonthrowing_class> v;
    try {
        v = throwing_class(helper);
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    }

    try {
        throwing_class tc(helper);
        v = tc;
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<throwing_class>(&v));
    }
}

inline void check_2(int helper = 1)
{
    boost::variant<nonthrowing_class, throwing_class> v;
    try {
        v = throwing_class(helper);
        BOOST_CHECK(v.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v));
    }

    try {
        throwing_class cl(helper);
        v = cl;
        BOOST_CHECK(v.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v));
    }
}

inline void check_3(int helper = 1)
{

    boost::variant<nonthrowing_class, throwing_class> v1, v2;

    swap(v1, v2);
    try {
        v1 = throwing_class(helper);
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
    }


    try {
        v2 = throwing_class(helper);
        BOOST_CHECK(v2.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v2));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v2));
    }


    if (!v1.which() && !v2.which()) {
        swap(v1, v2); // Make sure that two backup holders swap well
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<nonthrowing_class>(&v2));

        v1 = v2;
    }
}

inline void check_4(int helper = 1)
{
    // This one has a fallback
    boost::variant<int, throwing_class> v1, v2;

    swap(v1, v2);
    try {
        v1 = throwing_class(helper);
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<int>(&v1));
    }


    try {
        v2 = throwing_class(helper);
        BOOST_CHECK(v2.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v2));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<int>(&v2));
    }

    if (!v1.which() && !v2.which()) {
        swap(v1, v2);
        BOOST_CHECK(!v1.which());
        BOOST_CHECK(boost::get<int>(&v1));
        BOOST_CHECK(!v2.which());
        BOOST_CHECK(boost::get<int>(&v2));

        v1 = v2;
    }
}

inline void check_5(int helper = 1)
{
    boost::variant<nonthrowing_class, throwing_class> v1, v2;
    throwing_class throw_not_now;
    throw_not_now.trash = throwing_class::do_not_throw;
    v1 = throw_not_now;
    v2 = throw_not_now;

    boost::get<throwing_class>(v1).trash = 1;
    boost::get<throwing_class>(v2).trash = 1;

    try {
        v1 = throwing_class(helper);
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    }

    boost::get<throwing_class>(v1).trash = throwing_class::do_not_throw;
    boost::get<throwing_class>(v2).trash = throwing_class::do_not_throw;
    v1 = nonthrowing_class();
    v2 = nonthrowing_class();
    try {
        v1 = throwing_class(helper);
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 0);
        BOOST_CHECK(boost::get<nonthrowing_class>(&v1));
    }

    int v1_type = v1.which();
    int v2_type = v2.which();
    try {
        swap(v1, v2); // Make sure that backup holders swap well
        BOOST_CHECK(v1.which() == v2_type);
        BOOST_CHECK(v2.which() == v1_type);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == v1_type);
        BOOST_CHECK(v2.which() == v2_type);
    }
}

inline void check_6(int helper = 1)
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
        tc.trash = helper;
        v1 = tc;
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(boost::get<throwing_class>(&v1));
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
    boost::get<throwing_class>(v1).trash = helper;
    try {
        swap(v1, v2);
    } catch (const exception_on_assignment& /*e*/) {
        BOOST_CHECK(v1.which() == 1);
        BOOST_CHECK(v2.which() == 0);
        BOOST_CHECK(boost::get<throwing_class>(v1).trash == helper);
    }
}

int test_main(int , char* [])
{
    // throwing_class::throw_after_1 + 1  => throw on first assignment/construction
    // throwing_class::throw_after_1  => throw on second assignment/construction
    // throwing_class::throw_after_2  => throw on third assignment/construction
    // ...
    for (int i = throwing_class::throw_after_1 + 1; i != throwing_class::do_not_throw; --i) {
        check_1(i);
        check_2(i);
        check_3(i);
        check_4(i);
        check_5(i);
        check_6(i);
    }

    return boost::exit_success;
}
