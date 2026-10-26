// Copyright 2026 Treston Cuzzort
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or https://www.boost.org/LICENSE_1_0.txt)

#include <boost/variant.hpp>
#include <boost/variant/multivisitors.hpp>
#include <boost/core/lightweight_test.hpp>
#include <type_traits>
#include <utility>

template<class T>
int category(T&&)
{
    return !std::is_lvalue_reference<T&&>::value
        + 2 * std::is_const<typename std::remove_reference<T>::type>::value;
}

struct category_visitor : boost::static_visitor<int>
{
    template<class T> int operator()(T&& v) const
    { return category(std::forward<T>(v)); }
    template<class T, class U> int operator()(T&& a, U&& b) const
    { return category(std::forward<T>(a)) + 4 * category(std::forward<U>(b)); }
    template<class T, class U, class V> int operator()(T&& a, U&& b, V&& c) const
    { return category(std::forward<T>(a)) + 4 * category(std::forward<U>(b))
           + 16 * category(std::forward<V>(c)); }
};

#ifdef BOOST_VARIANT_HAS_DECLTYPE_APPLY_VISITOR_RETURN_TYPE
struct deduced_visitor
{
    template<class T> int operator()(T&& v) const
    { return category(std::forward<T>(v)); }
    template<class T, class U> int operator()(T&& a, U&& b) const
    { return category(std::forward<T>(a)) + 4 * category(std::forward<U>(b)); }
};

struct deduced_reference
{
    template<class T> T& operator()(T& v) const { return v; }
};
#endif

template<class Variant>
void check_categories(Variant& v, int lvalue, int rvalue)
{
    category_visitor visitor;
    BOOST_TEST_EQ(v.apply_visitor(visitor), lvalue);
    BOOST_TEST_EQ(std::move(v).apply_visitor(visitor), rvalue);
    BOOST_TEST_EQ(boost::apply_visitor(visitor, v), lvalue);
    BOOST_TEST_EQ(boost::apply_visitor(visitor, std::move(v)), rvalue);
#ifdef BOOST_VARIANT_HAS_DECLTYPE_APPLY_VISITOR_RETURN_TYPE
    deduced_visitor deduced;
    BOOST_TEST_EQ(boost::apply_visitor(deduced, v), lvalue);
    BOOST_TEST_EQ(boost::apply_visitor(deduced, std::move(v)), rvalue);
#endif
}

struct changer : boost::static_visitor<void>
{
    changer() {}
    changer(const changer&) = delete;
    changer& operator=(const changer&) = delete;
    void operator()(int& i) { i += 10; }
    void operator()(double& d) { d += 20; }
};

struct tracked
{
    static int copies, moves;
    int value;
    explicit tracked(int v) : value(v) {}
    tracked(const tracked& v) : value(v.value) { ++copies; }
    tracked(tracked&& v) : value(v.value) { ++moves; v.value = -1; }
};
int tracked::copies = 0;
int tracked::moves = 0;

struct consume : boost::static_visitor<int>
{
    int operator()(tracked value) const { return value.value; }
};

struct address : boost::static_visitor<tracked*>
{
    tracked* operator()(tracked& value) const { return &value; }
};

void test_categories()
{
    int i = 1;
    const int ci = 2;
    boost::variant<int> owned(i);
    const boost::variant<int> const_owned(i);
    boost::variant<int&> borrowed(i);
    const boost::variant<int&> const_borrowed(i);
    boost::variant<const int&> borrowed_const(ci);
    const boost::variant<const int&> const_borrowed_const(ci);
    check_categories(owned, 0, 1);
    check_categories(const_owned, 2, 3);
    check_categories(borrowed, 0, 0);
    check_categories(const_borrowed, 0, 0);
    check_categories(borrowed_const, 2, 2);
    check_categories(const_borrowed_const, 2, 2);

    category_visitor visitor;
    BOOST_TEST_EQ(boost::apply_visitor(visitor, std::move(borrowed), std::move(owned)), 4);
    BOOST_TEST_EQ(boost::apply_visitor(visitor, std::move(owned), std::move(borrowed)), 1);
    BOOST_TEST_EQ(boost::apply_visitor(visitor, std::move(const_borrowed), std::move(borrowed_const)), 8);
    BOOST_TEST_EQ(boost::apply_visitor(visitor, std::move(borrowed), std::move(owned), std::move(borrowed_const)), 36);
#ifdef BOOST_VARIANT_HAS_DECLTYPE_APPLY_VISITOR_RETURN_TYPE
    deduced_visitor deduced;
    BOOST_TEST_EQ(boost::apply_visitor(deduced, std::move(borrowed), std::move(owned)), 4);
    BOOST_TEST_EQ(boost::apply_visitor(deduced, std::move(owned), std::move(borrowed)), 1);
    deduced_reference reference;
    static_assert(std::is_same<decltype(boost::apply_visitor(reference, std::move(borrowed))), int&>::value,
                  "A borrowed alternative keeps its reference result");
    BOOST_TEST(&boost::apply_visitor(reference, std::move(borrowed)) == &i);
    BOOST_TEST(&boost::apply_visitor(reference, std::move(const_borrowed)) == &i);
#endif
}

void test_mutation_and_moves()
{
    int i = 0;
    double d = 0;
    typedef boost::variant<int&, double&> references;
    references v(i);
    const references cv(i);
    changer change;
    boost::apply_visitor(change, v);
    boost::apply_visitor(change, references(d));
    std::move(cv).apply_visitor(change);
    BOOST_TEST_EQ(i, 20);
    BOOST_TEST_EQ(d, 20);

    tracked external(42);
    boost::variant<tracked&> borrowed(external);
    const boost::variant<tracked&> const_borrowed(external);
    consume sink;
    tracked::copies = tracked::moves = 0;
    BOOST_TEST_EQ(boost::apply_visitor(sink, std::move(borrowed)), 42);
    BOOST_TEST_EQ(external.value, 42);
    BOOST_TEST_EQ(tracked::copies, 1);
    BOOST_TEST_EQ(tracked::moves, 0);
    tracked::copies = tracked::moves = 0;
    BOOST_TEST_EQ(boost::apply_visitor(sink, std::move(const_borrowed)), 42);
    BOOST_TEST_EQ(external.value, 42);
    BOOST_TEST_EQ(tracked::copies, 1);
    BOOST_TEST_EQ(tracked::moves, 0);

    boost::variant<tracked> owned(tracked(31));
    tracked::copies = tracked::moves = 0;
    BOOST_TEST_EQ(boost::apply_visitor(sink, std::move(owned)), 31);
    BOOST_TEST_EQ(boost::get<tracked>(owned).value, -1);
    BOOST_TEST_EQ(tracked::copies, 0);
    BOOST_TEST_EQ(tracked::moves, 1);
}

void test_recursive_references()
{
    boost::recursive_wrapper<tracked> wrapped(tracked(7));
    typedef boost::variant<boost::recursive_wrapper<tracked>&> references;
    references v(wrapped);
    const references cv(wrapped);
    address visitor;
    BOOST_TEST(boost::apply_visitor(visitor, v) == &wrapped.get());
    BOOST_TEST(boost::apply_visitor(visitor, std::move(v)) == &wrapped.get());
    BOOST_TEST(boost::apply_visitor(visitor, cv) == &wrapped.get());
    BOOST_TEST(boost::apply_visitor(visitor, std::move(cv)) == &wrapped.get());
    check_categories(v, 0, 0);
    check_categories(cv, 0, 0);
    boost::variant<const boost::recursive_wrapper<tracked>&> const_reference(wrapped);
    check_categories(const_reference, 2, 2);
    boost::variant<boost::recursive_wrapper<tracked> > owned(wrapped.get());
    check_categories(owned, 0, 1);
}

int main()
{
    test_categories();
    test_mutation_and_moves();
    test_recursive_references();
    return boost::report_errors();
}
