//-----------------------------------------------------------------------------
// boost-libs variant/test/auto_visitors.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------

#include "boost/config.hpp"

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"
#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/multivisitors.hpp"
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

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    construction_logger(construction_logger&& cl) :
        _val(cl._val)
    {
        std::cout << _val << " move constructed\n";
    }
#endif

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

    template <class T, class V, class P, class S>
    std::string operator()(const T& val, const V& val2, const P& val3, const S& val4) const
    {
        return lcs(val) + '+' + lcs(val2) + '+' + lcs(val3) + '+' + lcs(val4);
    }
};

struct lvalue_rvalue_detector : boost::static_visitor<std::string>
{
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    template <class T>
    std::string operator()(T&&) const
    {
        if (std::is_lvalue_reference<T>::value)
            return "lvalue reference";
        else
            return "rvalue reference";
    }

    template <class T, class V>
    std::string operator()(T&& t, V&& v) const
    {
        return operator()(std::forward<T>(t)) + ", " + operator()(std::forward<V>(v));
    }
#else
    template <class T>
    std::string operator()(T&) const
    {
        return "lvalue reference";
    }

    template <class T, class V>
    std::string operator()(T&, V&) const
    {
        return "lvalue reference, lvalue reference";
    }
#endif
};

typedef boost::variant<construction_logger, std::string> variant_type;

void test_const_ref_parameter(const variant_type& test_var)
{
    std::cout << "Testing const lvalue reference visitable\n";

    BOOST_CHECK(boost::apply_visitor(lvalue_rvalue_detector(), test_var) == "lvalue reference");
}

void test_const_ref_parameter2(const variant_type& test_var, const variant_type& test_var2)
{
    std::cout << "Testing const lvalue reference visitable\n";

    BOOST_CHECK(boost::apply_visitor(lvalue_rvalue_detector(), test_var, test_var2) == "lvalue reference, lvalue reference");
}

void test_const_ref_parameter4(const variant_type& test_var, const variant_type& test_var2, const variant_type& test_var3, const variant_type& test_var4)
{
    std::cout << "Testing const lvalue reference visitable with multivisitor\n";

    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), test_var, test_var2, test_var3, test_var4)
            == lcs(test_var) + '+' + lcs(test_var2) + '+' + lcs(test_var3) + '+' + lcs(test_var4));
}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES

void test_rvalue_parameter(variant_type&& test_var)
{
    std::cout << "Testing rvalue visitable\n";

    const auto expected_val = lcs(test_var);
    BOOST_CHECK(boost::apply_visitor(lvalue_rvalue_detector(), std::move(test_var)) == "rvalue reference");
}

void test_rvalue_parameter2(variant_type&& test_var, variant_type&& test_var2)
{
    std::cout << "Testing rvalue visitable\n";

    BOOST_CHECK(boost::apply_visitor(lvalue_rvalue_detector(), std::move(test_var), std::move(test_var2)) == "rvalue reference, rvalue reference");
}

void test_rvalue_parameter4(variant_type&& test_var, variant_type&& test_var2, variant_type&& test_var3, variant_type&& test_var4)
{
    std::cout << "Testing rvalue visitable with multivisitor\n";

    const auto expected_val = lcs(test_var) + '+' + lcs(test_var2) + '+' + lcs(test_var3) + '+' + lcs(test_var4);
    BOOST_CHECK(boost::apply_visitor(lex_streamer_explicit(), std::move(test_var), std::move(test_var2), std::move(test_var3), std::move(test_var4)) == expected_val);
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
    std::cout << "Testing rvalue visitable for c++14\n";

    const auto expected_val = lcs(test_var);
    BOOST_CHECK(boost::apply_visitor([](auto&& v) { return lcs(v); }, test_var) == expected_val);
}

void test_cpp14_visitor(variant_type&& test_var, variant_type&& test_var2)
{
    std::cout << "Testing rvalue visitable for c++14\n";

    const auto expected_val = lcs(test_var) + '+' + lcs(test_var2);
    BOOST_CHECK(boost::apply_visitor([](auto&& v, auto&& vv) { return lcs(v) + '+' + lcs(vv); }, std::move(test_var), std::move(test_var2)) == expected_val);
}

#endif

void run_const_lvalue_ref_tests()
{
        const variant_type v1(1), v2(2), v3(3), v4(4);
        test_const_ref_parameter(v1);
        test_const_ref_parameter2(v1, v2);
        test_const_ref_parameter4(v1, v2, v3, v4);
}

void run_rvalue_ref_tests()
{
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    variant_type v1(10), v2(20), v3(30);
    test_rvalue_parameter(boost::move(v1));
    test_rvalue_parameter2(boost::move(v2), boost::move(v3));

    variant_type vv1(100), vv2(200), vv3(300), vv4(400);
    test_rvalue_parameter4(boost::move(vv1), boost::move(vv2), boost::move(vv3), boost::move(vv4));
#endif
}

void run_cpp14_tests()
{
#ifndef BOOST_NO_CXX14_DECLTYPE_AUTO
    variant_type v1(10), v2(20), v3(30);

    test_cpp14_visitor(v1);
    test_cpp14_visitor(v2, v3);

    test_cpp14_visitor(boost::move(v1));
    test_cpp14_visitor(boost::move(v2), boost::move(v3));

    //lambda visitors doesn't support multivisotors
#endif
}

int test_main(int , char* [])
{
    run_const_lvalue_ref_tests();
    run_rvalue_ref_tests();
    run_cpp14_tests();

    return 0;
}
