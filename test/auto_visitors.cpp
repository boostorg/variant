//-----------------------------------------------------------------------------
// boost-libs variant/test/auto_visitors.cpp source file
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
#include "boost/variant/multivisitors.hpp"
#include "boost/lexical_cast.hpp"

struct lex_streamer_explicit: boost::static_visitor<std::string> {
    template <class T>
    const char* operator()(const T& ) {
        return "10";
    }

    template <class T1, class T2>
    const char* operator()(const T1& , const T2& ) {
        return "100";
    }
};


void run_explicit()
{
    typedef boost::variant<int, std::string, double> variant_type;
    variant_type v2("10"), v1("100");

    lex_streamer_explicit visitor_ref;

    // Must return instance of std::string
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2).c_str() == std::string("10"));
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2, v1).c_str() == std::string("100"));
}


// Most part of tests from this file require decltype(auto)

#ifdef BOOST_NO_CXX14_DECLTYPE_AUTO

void run()
{
    BOOST_CHECK(true);
}

void run2()
{
    BOOST_CHECK(true);
}


void run3()
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


    template <class T1, class T2>
    void operator()(const T1& val, const T2& val2) const {
        std::cout << val << '+' << val2 << std::endl;
    }


    template <class T1, class T2, class T3>
    void operator()(const T1& val, const T2& val2, const T3& val3) const {
        std::cout << val << '+' << val2 << '+' << val3 << std::endl;
    }
};


struct lex_streamer2 {
    std::string res;

    template <class T>
    const char* operator()(const T& val) const {
        return "fail";
    }

    template <class T1, class T2>
    const char* operator()(const T1& v1, const T2& v2) const {
        return "fail2";
    }


    template <class T1, class T2, class T3>
    const char* operator()(const T1& v1, const T2& v2, const T3& v3) const {
        return "fail3";
    }

    template <class T>
    std::string& operator()(const T& val) {
        res = boost::lexical_cast<std::string>(val);
        return res;
    }


    template <class T1, class T2>
    std::string& operator()(const T1& v1, const T2& v2) {
        res = boost::lexical_cast<std::string>(v1) + "+" + boost::lexical_cast<std::string>(v2);
        return res;
    }


    template <class T1, class T2, class T3>
    std::string& operator()(const T1& v1, const T2& v2, const T3& v3) {
        res = boost::lexical_cast<std::string>(v1) + "+" + boost::lexical_cast<std::string>(v2) 
            + "+" + boost::lexical_cast<std::string>(v3);
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


struct lex_combine {
    template <class T1, class T2>
    std::string operator()(const T1& v1, const T2& v2) const {
        return boost::lexical_cast<std::string>(v1) + "+" + boost::lexical_cast<std::string>(v2);
    }


    template <class T1, class T2, class T3>
    std::string operator()(const T1& v1, const T2& v2, const T3& v3) const {
        return boost::lexical_cast<std::string>(v1) + "+" 
                + boost::lexical_cast<std::string>(v2) + '+'
                + boost::lexical_cast<std::string>(v3);
    }
};

void run2()
{
    typedef boost::variant<int, std::string, double> variant_type;
    variant_type v1(1), v2("10"), v3(100.0);

    BOOST_CHECK(boost::apply_visitor(lex_combine(), v1, v2) == "1+10");
    BOOST_CHECK(boost::apply_visitor(lex_combine(), v2, v1) == "10+1");


    #ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
        BOOST_CHECK(
            boost::apply_visitor(
                [](auto v1, auto v2) {
                    return boost::lexical_cast<std::string>(v1) + "+"
                        + boost::lexical_cast<std::string>(v2);
                }
                , v1
                , v2
            ) == "1+10"
        );
        BOOST_CHECK(
            boost::apply_visitor(
                [](auto v1, auto v2) {
                    return boost::lexical_cast<std::string>(v1) + "+"
                        + boost::lexical_cast<std::string>(v2);
                }
                , v2
                , v1
            ) == "10+1"
        );

        boost::apply_visitor([](auto v1, auto v2) { std::cout << v1 << '+' << v2 << std::endl; }, v1, v2);
        boost::apply_visitor([](auto v1, auto v2) { std::cout << v1 << '+' << v2 << std::endl; }, v2, v1);
    #endif


    lex_streamer2 visitor_ref;
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v1, v2) == "1+10");
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2, v1) == "10+1");
    std::string& ref_to_string = boost::apply_visitor(visitor_ref, v1, v2);
    BOOST_CHECK(ref_to_string == "1+10");

    boost::apply_visitor(lex_streamer_void(), v1, v2);
    boost::apply_visitor(lex_streamer_void(), v2, v1);
}

void run3()
{
    typedef boost::variant<int, std::string, double> variant_type;
    variant_type v1(1), v2("10"), v3(100);

    BOOST_CHECK(boost::apply_visitor(lex_combine(), v1, v2, v3) == "1+10+100");
    BOOST_CHECK(boost::apply_visitor(lex_combine(), v2, v1, v3) == "10+1+100");


    #ifndef BOOST_NO_CXX14_GENERIC_LAMBDAS
        BOOST_CHECK(
            boost::apply_visitor(
                [](auto v1, auto v2, auto v3) {
                    return boost::lexical_cast<std::string>(v1) + "+"
                        + boost::lexical_cast<std::string>(v2) + "+"
                        + boost::lexical_cast<std::string>(v3);
                }
                , v1
                , v2
                , v3
            ) == "1+10+100"
        );
        BOOST_CHECK(
            boost::apply_visitor(
                [](auto v1, auto v2, auto v3) {
                    return boost::lexical_cast<std::string>(v1) + "+"
                        + boost::lexical_cast<std::string>(v2) + "+"
                        + boost::lexical_cast<std::string>(v3);
                }
                , v3
                , v1
                , v3
            ) == "100+1+100"
        );

        boost::apply_visitor(
            [](auto v1, auto v2, auto v3) { std::cout << v1 << '+' << v2 << '+' << v3 << std::endl; }, 
            v1, v2, v3
        );
        boost::apply_visitor(
            [](auto v1, auto v2, auto v3) { std::cout << v1 << '+' << v2 << '+' << v3 << std::endl; },
            v2, v1, v3
        );
    #endif


    lex_streamer2 visitor_ref;
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v1, v2) == "1+10");
    BOOST_CHECK(boost::apply_visitor(visitor_ref, v2, v1) == "10+1");
    std::string& ref_to_string = boost::apply_visitor(visitor_ref, v1, v2);
    BOOST_CHECK(ref_to_string == "1+10");

    boost::apply_visitor(lex_streamer_void(), v1, v2, v1);
    boost::apply_visitor(lex_streamer_void(), v2, v1, v1);
}
#endif


int test_main(int , char* [])
{
    run_explicit();
    run();
    run2();

    return 0;
}
