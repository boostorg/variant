//-----------------------------------------------------------------------------
// boost variant_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
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
#include "boost/visitor/apply_visitor.hpp"
#include "boost/visitor/static_visitor.hpp"
#include "boost/visitor/visitor_ptr.hpp"

// Support headers:
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "boost/test/minimal.hpp"

//////////////////////////////////////////////////////////////////////////

typedef boost::variant<
      int
    , double
    , std::string
    > my_variant;

class display_visitor
    : public boost::static_visitor<>
{
private: // representation

    std::ostream* o_;

public: //structors

    explicit display_visitor(std::ostream& o)
        : o_(&o)
    { }

public: // visitor interfaces

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
    : public boost::static_visitor<int>
{
private: // representation

    int value_;

public: // structors

    explicit calculator(int value)
        : value_(value)
    {
    }

public: // visitor interfaces

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
class are_strict_equals
    : public boost::static_visitor<bool>
{
public: // visitor interfaces

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
    : public boost::static_visitor<bool>
{
private: // representation

    T value_;

public: // structors

    explicit strict_equal_to(const T& value)
        : value_(value)
    {
    }

public: // visitor interfaces

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
    // initialize, copy, assign, and swap tests
    {
        my_variant def;
        my_variant var(2.71);
        my_variant varcopy(var);
        def = var;
        def = var; // check to ensure assign works

        BOOST_TEST((
              boost::apply_visitor(are_strict_equals(), var, varcopy)
            ));
        BOOST_TEST((
              boost::apply_visitor(are_strict_equals(), var, def)
            ));
        BOOST_TEST((
              boost::apply_visitor(are_strict_equals(), def, varcopy)
            ));

        my_variant swapped;
        swapped.swap(varcopy);

        BOOST_TEST((
              boost::apply_visitor(are_strict_equals(), swapped, var)
            ));
    }

    // extract<T> tests
    {
        double d = 12.345;
        my_variant var(d);

        boost::extract<int> xi(var);
        BOOST_TEST((
              xi.check() == false
            ));

        boost::extract<double> xd(var);
        BOOST_TEST((
              xd.check() == true
            ));
        BOOST_TEST((
              xd() == d
            ));

        BOOST_TEST((
              boost::extract<double>(var) == d
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

        std::cout << "\nexpected int = 42; actual = ";
        boost::apply_visitor(boost::visitor_ptr(&handle_int), var);
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

    std::cout << '\n';
    return boost::exit_success;
}
