//-----------------------------------------------------------------------------
// boost-libs variant/test/auto_visitors.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------

#include "boost/config.hpp"

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"
#include "boost/variant/apply_visitor.hpp"
#include "boost/lexical_cast.hpp"

#define lcs(val) boost::lexical_cast<std::string>(val)

struct construction_logger
{
    int _val;

    construction_logger(int val) : _val(val)
    {
        std::cout << _val << " constructed\n";
    }

    construction_logger(const construction_logger& cl) :
        _val(cl._val)
    {
        std::cout << _val << " copy constructed\n";
    }

    construction_logger(construction_logger&& cl) :
        _val(cl._val)
    {
        std::cout << _val << " move constructed\n";
    }

    friend std::ostream& operator << (std::ostream& os, const construction_logger& cl)
    {
        return os << cl._val;
    }

    friend std::istream& operator << (std::istream& is, construction_logger& cl)
    {
        return is >> cl._val;
    }
};

struct lex_streamer_explicit : boost::static_visitor<std::string>
{
    template <class T>
    std::string operator()(const T& val) const
    {
        return lcs(val);
    }

    template <class T, class V>
    std::string operator()(const T& val, const V& val2) const
    {
        return lcs(val) + '+' + lcs(val2);
    }
};

typedef boost::variant<construction_logger, std::string> variant_type;

void test_const_ref_parameter(const variant_type& test_var)
{
    std::cout << "Testing const lvalue reference visitable\n";

    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), test_var) == lcs(test_var));
}

void test_const_ref_parameter2(const variant_type& test_var, const variant_type& test_var2)
{
    std::cout << "Testing const lvalue reference visitable\n";

    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), test_var, test_var2) == lcs(test_var) + '+' + lcs(test_var2));
}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES

void test_rvalue_parameter(variant_type&& test_var)
{
    std::cout << "Testing rvalue visitable\n";

    const auto expected_val = lcs(test_var);
    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), std::move(test_var)) == expected_val);
}

void test_rvalue_parameter2(variant_type&& test_var, variant_type&& test_var2)
{
    std::cout << "Testing rvalue visitable\n";

    const auto expected_val = lcs(test_var) + '+' + lcs(test_var2);
    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), std::move(test_var), std::move(test_var2)) == expected_val);
}

#endif

#ifndef BOOST_NO_CXX14_DECLTYPE_AUTO

void test_cpp14_visitor(const variant_type& test_var)
{
    std::cout << "Testing const lvalue visitable for c++14\n";

    BOOST_CHECK(boost::apply_visitor([](auto&& v) { return lcs(v); }, test_var) == lcs(test_var));
}

void test_cpp14_visitor(const variant_type& test_var, const variant_type& test_var2)
{
    std::cout << "Testing const lvalue visitable for c++14\n";

    BOOST_CHECK(boost::apply_visitor([](auto&& v, auto&& vv) { return lcs(v) + '+' + lcs(vv); }, test_var, test_var2) == lcs(test_var) + '+' + lcs(test_var2));
}

void test_cpp14_visitor(variant_type&& test_var)
{
    std::cout << "Testing const rvalue visitable for c++14\n";

    const auto expected_val = lcs(test_var);
    BOOST_CHECK(boost::apply_visitor([](auto&& v) { return lcs(v); }, test_var) == expected_val);
}

void test_cpp14_visitor(variant_type&& test_var, variant_type&& test_var2)
{
    std::cout << "Testing const rvalue visitable for c++14\n";

    const auto expected_val = lcs(test_var) + '+' + lcs(test_var2);
    BOOST_CHECK(boost::apply_visitor([](auto&& v, auto&& vv) { return lcs(v) + '+' + lcs(vv); }, std::move(test_var), std::move(test_var2)) == expected_val);
}

#endif

void run()
{
    {
        const variant_type v1(1), v2(2);
        test_const_ref_parameter(v1);
        test_const_ref_parameter2(v1, v2);
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    {
        variant_type v1(10), v2(20), v3(30);
        test_rvalue_parameter(boost::move(v1));
        test_rvalue_parameter2(boost::move(v2), boost::move(v3));
    }
#endif

#ifndef BOOST_NO_CXX14_DECLTYPE_AUTO
    {
        variant_type v1(10), v2(20), v3(30);

        test_cpp14_visitor(v1);
        test_cpp14_visitor(v2, v3);

        test_cpp14_visitor(boost::move(v1));
        test_cpp14_visitor(boost::move(v2), boost::move(v3));
    }
#endif

}

int test_main(int , char* [])
{
    run();

    return 0;
}
