//-----------------------------------------------------------------------------
// boost variant_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002
// Eric Friedman
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee, 
// provided that the above copyright notice appears in all copies and 
// that both the copyright notice and this permission notice appear in 
// supporting documentation. No representations are made about the 
// suitability of this software for any purpose. It is provided "as is" 
// without express or implied warranty.

// Headers whose facilities are to be tested:
#include "boost/variant.hpp"
#include "boost/extract.hpp"
#include "boost/incomplete.hpp"
#include "boost/type_switch.hpp"
#include "boost/apply_visitor.hpp"

// Support headers:
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#define BOOST_INCLUDE_MAIN
#include <boost/test/test_tools.hpp>
#include "boost/cstdlib.hpp"

//////////////////////////////////////////////////////////////////////////

typedef boost::variant<
      int
    , double
    , std::string
    > my_variant;

struct display_visitor
    : boost::generic_visitor<>
{
private:
    std::ostream* o_;

public:
    explicit display_visitor(std::ostream& o)
        : o_(&o)
    { }

    template <typename T>
    void operator()(const T& operand)
    {
        *o_ << typeid(T).name() << '(' << operand << ')';
    }

    void operator()(const std::string& val)
    {
        *o_ << val;
    }
};

template <typename T, typename Variant>
bool can_extract_ptr(const Variant* pvar)
{
    return (boost::extract<const T>(pvar) != 0);
}

template <typename T, typename Variant>
bool can_extract_ref(const Variant& var)
{
    try
    {
        const T& r = boost::extract<const T>(var);
        return true;
    }
    catch (const boost::bad_extract&)
    {
        return false;
    }
}

std::vector<my_variant> create_values()
{
    std::vector<my_variant> ret;

    ret.reserve(5);
    ret.push_back("pi rounded to six places is ");
    ret.push_back(3.14159);
    ret.push_back(", but the Romans sometimes rounded it to ");
    ret.push_back(4);
    ret.push_back("!");

    return ret;
}

void switch_default()
{
    std::cout << "***default handler";
}

void handle_int(const int& i)
{
    std::cout << "int(" << i << ')';
}

struct handle_double
{
    void operator()(const double& d)
    {
        std::cout << "double(" << d << ')';
    }
};

//////////////////////////////////////////////////////////////////////////
// incomplete types example (thanks to Itay Maman)

struct variable { };
struct add;
struct subtract;

typedef boost::variant<
      int
    , variable
    , boost::incomplete<add>
    , boost::incomplete<subtract>
    > expression;

struct add
{
    expression lhs;
    expression rhs;

    add(const expression& lhs_init, const expression& rhs_init)
        : lhs(lhs_init)
        , rhs(rhs_init)
    { }
};

struct subtract
{
    expression lhs;
    expression rhs;

    subtract(const expression& lhs_init, const expression& rhs_init)
        : lhs(lhs_init)
        , rhs(rhs_init)
    { }
};

class calculator
    : boost::generic_visitor<int>
{
    int value_;

public:
    explicit calculator(int value)
        : value_(value)
    {
    }

    int operator()(variable) const
    {
        return value_;
    }

    int operator()(int x) const
    {
        return x;
    }

    int operator()(const add& x) const
    {
        return boost::apply_visitor(*this, x.lhs) + boost::apply_visitor(*this, x.rhs);
    }

    int operator()(const subtract& x) const
    {
        return boost::apply_visitor(*this, x.lhs) - boost::apply_visitor(*this, x.rhs);
    }
};

//////////////////////////////////////////////////////////////////////////
// class are_strict_equals
//
struct are_strict_equals
    : boost::generic_visitor<bool>
{

#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

    template <typename T>
    bool operator()(const T& x, const T& y) const
    {
        return x == y;
    }

    template <typename T, typename U>
    bool operator()(const T&, const U&) const
    {
        return false;
    }

#else// defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

private:
    template <typename T>
    bool execute_impl(const T& x, const T& y, long) const
    {
        return x == y;
    }

    template <typename T, typename U>
    bool execute_impl(const T&, const U&, int) const
    {
        return false;
    }

public:
    template <typename T, typename U>
    bool operator()(const T& x, const U& y) const
    {
        return execute_impl(x, y, 1L);
    }

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING workaround

};

//////////////////////////////////////////////////////////////////////////
// class template strict_equal_to
//
template <typename T>
class strict_equal_to
    : boost::generic_visitor<bool>
{
    T value_;

public:
    explicit strict_equal_to(const T& value)
        : value_(value)
    {
    }

    bool operator()(const T& operand) const
    {
        return operand == value_;
    }

    template <typename U>
    bool operator()(const U&) const
    {
        return false;
    }
};

//////////////////////////////////////////////////////////////////////////
// function test_main
//
int test_main( int, char *[] )
{
    // initialize, assign, and copy tests
    {
        my_variant var(2.71);
        my_variant varcopy(var);

        BOOST_TEST((
              boost::apply_visitor(are_strict_equals(), var, varcopy)
            ));
    }

    // extract<T> tests
    {
        my_variant var(12.345);

        BOOST_TEST((
              can_extract_ptr<int>(&var) == false
            ));
        BOOST_TEST((
              can_extract_ref<int>(var) == false
            ));

        BOOST_TEST((
              can_extract_ptr<double>(&var) == true
            ));
        BOOST_TEST((
              can_extract_ref<double>(var) == true
            ));
    }

    // static-converting-copy tests
    {
        boost::variant<int, float> v1(3);
        boost::variant<double, std::string> v2(v1);

        BOOST_TEST((
              boost::apply_visitor(strict_equal_to<double>(3), v2)
            ));
    }

    // recursive variant tests
    {
        expression expr( // 10 + (40 + 2) - (4 + x) = 48 - x
            add(
                  10
                , subtract(
                      add(40, 2)
                    , add(4, variable())
                    )
                )
            );
        static const int n = 13;

        // result should be 48 - n
        BOOST_TEST((
              boost::apply_visitor(calculator(n), expr) == 48 - n
            ));
    }

    //////////////////////////////////////////////////////////////////////
    std::cout << "__boost::variant tests__";

    display_visitor displayer(std::cout);

    // visitor_ptr/apply_visitor (w/ ptr) tests
    {
        std::cout << "\n\n* variant apply visitor_ptr tests:\n";

        my_variant var(42);

        std::cout << "\nexplicit visitor_ptr() use: ";
        boost::apply_visitor(boost::visitor_ptr(&handle_int), var);
        std::cout << "\nimplicit: ";
        boost::apply_visitor(&handle_int, var);
    }

    // type_switch tests
    {
        std::cout << "\n\n* type_switch tests:\n";

        my_variant var(42);
        using namespace boost::type_switch;

        std::cout << "\n - strict switch: ";
        switch_(var)[
              case_<int>(&handle_int) // function pointer
            | case_<double>(handle_double()) // function object
            | case_<std::string>() // ignore string
            ];

        std::cout << "\n - non-strict switch: ";
        switch_(var)[
              case_<double>(handle_double()) // function object
            | case_<std::string>() // ignore string
            | default_(&switch_default) // default
            ];

        std::cout << "\n - templated switch: ";
        switch_(var)[
              case_<std::string>() // ignore string
            | template_(displayer) // display else
            ];
    }

    // vector test
    {
        std::cout << "\n\n* variant vector test:\n";

        std::vector<my_variant> vec = create_values();
        std::for_each(
              vec.begin()
            , vec.end()
            , boost::apply_visitor(displayer)
            );
    }

    std::cout << "\n\n";
    return boost::exit_success;
}
