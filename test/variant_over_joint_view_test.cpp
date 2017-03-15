// Copyright (c) 2017
// Mikhail Maximov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// The test is base on https://svn.boost.org/trac/boost/ticket/8554 
// variant was not able to extract types from mpl::joint_view

#include <string>

#include "boost/config.hpp"
#include "boost/test/minimal.hpp"

#include "boost/variant.hpp"
#include "boost/mpl/joint_view.hpp"
#include "boost/mpl/insert_range.hpp"

typedef boost::variant<int> v1;
typedef boost::variant<std::string> v2;
typedef boost::make_variant_over<boost::mpl::joint_view<v1::types, v2::types>::type>::type v3;

template<class T>
void check_exception_on_get(v3& v) {
    try {
        boost::get<T>(v);
        BOOST_FAIL("Expected exception boost::bad_get, but got nothing.");
    } catch (boost::bad_get& exc) { //okay it is expected behaviour
    } catch (...) { BOOST_FAIL("Expected exception boost::bad_get, but got something else."); }
}

int test_main(int , char* [])
{
    v1 a = 1;
    v2 b = "2";
    v3 c = a;
    BOOST_CHECK(boost::get<int>(c) == 1);
    v3 d = b;
    BOOST_CHECK(boost::get<std::string>(d) == "2");
    check_exception_on_get<std::string>(c);
    check_exception_on_get<int>(d);
    return 0;
}