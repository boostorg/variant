//-----------------------------------------------------------------------------
// boost-libs variant/test/rvalue_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2014 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/config.hpp"

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"
#include "boost/lexical_cast.hpp"

struct lex_streamer_explicit: boost::static_visitor<std::string> {
    template <class T>
    const char* operator()(const T& val) {
        return "10";
    }
};


void run_explicit()
{
    typedef boost::variant<int, std::string, double> variant_type;
    variant_type v2("10");

    lex_streamer_explicit visitor_ref;

    // Must return instance of std::string
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2).c_str() == std::string("10"));
}


// Most part of tests from this file require decltype(auto)

#ifdef BOOST_NO_CXX14_DECLTYPE_AUTO

void run()
{
    BOOST_CHECK(true);
}


#else

#include <iostream>

struct lex_streamer {
    template <class T>
    std::string operator()(const T& val) const {
        return boost::lexical_cast<std::string>(val);
    }
};

struct lex_streamer_void {
    template <class T>
    void operator()(const T& val) const {
        std::cout << val << std::endl;
    }
};


struct lex_streamer2 {
    std::string res;

    template <class T>
    const char* operator()(const T& val) const {
        return "fail";
    }

    template <class T>
    std::string& operator()(const T& val) {
        res = boost::lexical_cast<std::string>(val);
        return res;
    }
};

void run()
{
    typedef boost::variant<int, std::string, double> variant_type;
    variant_type v1(1), v2("10"), v3(100.0);

    BOOST_CHECK(boost::apply_visitor(lex_streamer(), v1) == "1");
    BOOST_CHECK(boost::apply_visitor(lex_streamer(), v2) == "10");

    #ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
        BOOST_CHECK(boost::apply_visitor([](auto v) { return boost::lexical_cast<std::string>(v); }, v1) == "1");
        BOOST_CHECK(boost::apply_visitor([](auto v) { return boost::lexical_cast<std::string>(v); }, v2) == "10");

        boost::apply_visitor([](auto v) { std::cout << v << std::endl; }, v1);
        boost::apply_visitor([](auto v) { std::cout << v << std::endl; }, v2);
    #endif

    lex_streamer2 visitor_ref;
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v1) == "1");
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2) == "10");
    std::string& ref_to_string = boost::apply_visitor(visitor_ref, v1);
    BOOST_CHECK(ref_to_string == "1");

    boost::apply_visitor(lex_streamer_void(), v1);
    boost::apply_visitor(lex_streamer_void(), v2);
}
#endif


int test_main(int , char* [])
{
    run_explicit();
    run();

    return 0;
}
