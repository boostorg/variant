// Copyright (c) 2017
// Mikhail Maximov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_ALTERNATIVE_HPP
#define BOOST_VARIANT_ALTERNATIVE_HPP

#include <boost/config.hpp>

#include <boost/mpl/at.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_cv.hpp>
#include <boost/type_traits/add_volatile.hpp>
#include <boost/type_traits/conditional.hpp> 

namespace boost {
namespace detail {
namespace variant { 

// copies sequence to mpl::list, then uses mpl::at to get I-th type
template <size_t I, class Sequence>
struct type_in_seqeunce {
 private:
  typedef typename boost::mpl::insert_range<
      boost::mpl::list<>
    , boost::mpl::end< boost::mpl::list<> >::type
    , typename Sequence::type
    >::type sequence;
 public:
  typedef typename mpl::at<
                    sequence,
                    mpl::integral_c<size_t, I> >::type type;
};

#if !defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)
// variant_alternative_impl using variadic templates

template <size_t I, class Type, class... Types> 
struct get_type {
  typedef typename get_type<I - 1, Types...>::type type;
};

template <class Type, class... Types>
struct get_type<0, Type, Types...> {
  typedef Type type;
};

template <size_t I, class... Types> 
struct variant_alternative_impl {
  typedef typename get_type<I, Types...>::type type;
};

// variant_alternative_impl specialization for make_variant_over_sequence
template <size_t I, class Sequence>
struct variant_alternative_impl<I, over_sequence<Sequence> > {
  typedef typename type_in_seqeunce<I, Sequence>::type type;
};

#else // defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)
// variant_alternative_impl without variadic templates using BOOST_VARIANT_ENUM_PARAMS

template <size_t I, BOOST_VARIANT_ENUM_PARAMS(class Types)> 
struct variant_alternative_impl: 
    mpl::at<
        mpl::vector<BOOST_VARIANT_ENUM_PARAMS(Types)>, 
        mpl::integral_c<size_t, I> > {};

// variant_alternative_impl specialization for make_variant_over_sequence
template <size_t I, class Sequence, BOOST_VARIANT_ENUM_SHIFTED_PARAMS(class T)> 
struct variant_alternative_impl<I, detail::variant::over_sequence<Sequence>, BOOST_VARIANT_ENUM_SHIFTED_PARAMS(T)> {
  typedef typename type_in_seqeunce<I, Sequence>::type type;
};

#endif

} // namespace variant
} // namespace detail

template <size_t I, class T> struct variant_alternative; // undefined
 
#if !defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)
// variant_alternative using variadic templates

template <size_t I, class... Types>
struct variant_alternative<I, variant<Types...>> {
  typedef typename detail::variant::variant_alternative_impl<I, Types...>::type type;
};

#else // defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)
// variant_alternative without variadic_templates using BOOST_VARIANT_ENUM_PARAMS

template <size_t I, BOOST_VARIANT_ENUM_PARAMS(class Types)>
struct variant_alternative<I, variant<BOOST_VARIANT_ENUM_PARAMS(Types) > > {
  typedef typename detail::variant::variant_alternative_impl<I, BOOST_VARIANT_ENUM_PARAMS(Types)>::type type;
};

#endif

// cv-qualifiers handlers
template <size_t I, class T> struct variant_alternative<I, const T> {
  typedef typename add_const<typename variant_alternative<I, T>::type>::type type;
};

template <size_t I, class T> struct variant_alternative<I, volatile T> {
  typedef typename add_volatile<typename variant_alternative<I, T>::type>::type type;
};

template <size_t I, class T> struct variant_alternative<I, const volatile T> {
  typedef typename add_cv<typename variant_alternative<I, T>::type>::type type;
};

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
template <size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;
#endif
  
} // namespace boost

#endif // BOOST_VARIANT_ALTERNATIVE_HPP