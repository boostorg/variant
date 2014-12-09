//-----------------------------------------------------------------------------
// boost-libs variant/test/variant_get_test.cpp source file
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
#include "boost/variant/recursive_wrapper.hpp"
#include "boost/test/minimal.hpp"

struct base {int trash;};
struct derived1 : base{};
struct derived2 : base{};

struct vbase { short trash; virtual ~vbase(){} virtual int foo() const { return 0; }  };
struct vderived1 : virtual vbase{ virtual int foo() const { return 1; } };
struct vderived2 : virtual vbase{ virtual int foo() const { return 3; } };
struct vderived3 : vderived1, vderived2 { virtual int foo() const { return 3; } };

typedef boost::variant<int, base, derived1, derived2, std::string> var_t;
typedef boost::variant<int&, base&, derived1&, derived2&, std::string&> var_ref_t;
typedef boost::variant<const int&, const base&, const derived1&, const derived2&, const std::string&> var_cref_t;

struct recursive_structure;
typedef boost::variant<
    int, base, derived1, derived2, std::string, boost::recursive_wrapper<recursive_structure>
> var_req_t;
struct recursive_structure { var_req_t var; };

template <class T, class V>
void check_get_on_types_impl(V* v) 
{
    if (!!boost::is_same<T, int>::value) {
        BOOST_CHECK(boost::get<int>(v));
        BOOST_CHECK(boost::get<const int>(v));
    } else {
        BOOST_CHECK(!boost::get<int>(v));
        BOOST_CHECK(!boost::get<const int>(v));
    }

    if (!!boost::is_same<T, base>::value) {
        BOOST_CHECK(boost::get<base>(v));
        BOOST_CHECK(boost::get<const base>(v));
    } else {
        BOOST_CHECK(!boost::get<base>(v));
        BOOST_CHECK(!boost::get<const base>(v));
    }

    if (!!boost::is_same<T, derived1>::value) {
        BOOST_CHECK(boost::get<derived1>(v));
        BOOST_CHECK(boost::get<const derived1>(v));
    } else {
        BOOST_CHECK(!boost::get<derived1>(v));
        BOOST_CHECK(!boost::get<const derived1>(v));
    }

    if (!!boost::is_same<T, derived2>::value) {
        BOOST_CHECK(boost::get<derived2>(v));
        BOOST_CHECK(boost::get<const derived2>(v));
    } else {
        BOOST_CHECK(!boost::get<derived2>(v));
        BOOST_CHECK(!boost::get<const derived2>(v));
    }

    if (!!boost::is_same<T, std::string>::value) {
        BOOST_CHECK(boost::get<std::string>(v));
        BOOST_CHECK(boost::get<const std::string>(v));
    } else {
        BOOST_CHECK(!boost::get<std::string>(v));
        BOOST_CHECK(!boost::get<const std::string>(v));
    }
    
    // Never exist in here
    //BOOST_CHECK(!boost::get<short>(v));
    //BOOST_CHECK(!boost::get<const short>(v));

    boost::get<T>(*v); // Must compile
    boost::get<const T>(*v); // Must compile
}

template <class T, class V>
void check_get_on_types(V* v) 
{
    check_get_on_types_impl<T, V>(v);
    check_get_on_types_impl<T, const V>(v);
}

void get_test()
{
    var_t v;
    check_get_on_types<int>(&v);

    var_t(base()).swap(v);
    check_get_on_types<base>(&v);

    var_t(derived1()).swap(v);
    check_get_on_types<derived1>(&v);

    var_t(derived2()).swap(v);
    check_get_on_types<derived2>(&v);

    var_t(std::string("Hello")).swap(v);
    check_get_on_types<std::string>(&v);
}

void get_ref_test()
{
    int i = 0;
    var_ref_t v(i);
    check_get_on_types<int>(&v);

    base b;
    var_ref_t v1(b);
    check_get_on_types<base>(&v1);

    derived1 d1;
    var_ref_t v2(d1);
    check_get_on_types<derived1>(&v2);

    derived2 d2;
    var_ref_t v3(d2);
    check_get_on_types<derived2>(&v3);

    std::string s("Hello");
    var_ref_t v4(s);
    check_get_on_types<std::string>(&v4);
}


void get_cref_test()
{
    int i = 0;
    var_cref_t v(i);
    BOOST_CHECK(boost::get<const int>(&v));
    BOOST_CHECK(!boost::get<const base>(&v));

    base b;
    var_cref_t v1(b);
    BOOST_CHECK(boost::get<const base>(&v1));
    BOOST_CHECK(!boost::get<const derived1>(&v1));
    BOOST_CHECK(!boost::get<const int>(&v1));

    std::string s("Hello");
    const var_cref_t v4 = s;
    BOOST_CHECK(boost::get<const std::string>(&v4));
    BOOST_CHECK(!boost::get<const int>(&v4));
}

void get_recursive_test()
{
    var_req_t v;
    check_get_on_types<int>(&v);

    var_req_t(base()).swap(v);
    check_get_on_types<base>(&v);

    var_req_t(derived1()).swap(v);
    check_get_on_types<derived1>(&v);

    var_req_t(derived2()).swap(v);
    check_get_on_types<derived2>(&v);

    var_req_t(std::string("Hello")).swap(v);
    check_get_on_types<std::string>(&v);

    recursive_structure s = { v }; // copying "v"
    v = s;
    check_get_on_types<recursive_structure>(&v);
}

int test_main(int , char* [])
{
    get_test();
    get_ref_test();
    get_cref_test();
    get_recursive_test();

    return boost::exit_success;
}
