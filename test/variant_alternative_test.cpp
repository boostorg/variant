// Copyright (c) 2017
// Mikhail Maximov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <string>

#include "boost/config.hpp"

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"
#include "boost/variant/variant_alternative.hpp"

#include "boost/type_traits/add_const.hpp"
#include "boost/type_traits/add_cv.hpp"
#include "boost/type_traits/add_volatile.hpp"
#include "boost/type_traits/integral_constant.hpp"
#include "boost/type_traits/is_const.hpp"
#include "boost/type_traits/is_volatile.hpp"
#include "boost/type_traits/remove_cv.hpp"

template<bool B1, bool B2>
struct logical_and: boost::false_type {};

template<>
struct logical_and<true, true>: boost::true_type {};

template<class T>
struct is_cv : logical_and<boost::is_const<T>::value, boost::is_volatile<T>::value> {};

void test_correct_type() {
  typedef boost::variant<int, std::string> variant_t;
  typedef typename boost::variant_alternative<0, variant_t>::type first_type;
  typedef typename boost::variant_alternative<1, variant_t>::type second_type;
  variant_t v;
  v = 1;
  BOOST_CHECK(boost::get<first_type>(v) == 1);
  v = "2";
  BOOST_CHECK(boost::get<second_type>(v) == "2");
}

void test_correct_alias_type() {
#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
  typedef boost::variant<int, std::string> variant_t;
  typedef typename boost::variant_alternative_t<0, variant_t> first_type;
  typedef typename boost::variant_alternative_t<1, variant_t> second_type;
  variant_t v;
  v = 1;
  BOOST_CHECK(boost::get<first_type>(v) == 1);
  v = "2";
  BOOST_CHECK(boost::get<second_type>(v) == "2");
#endif
}

template<template <typename> class cv_qualifier, template <typename> class cv_check>
void test_correct_cv() {
  typedef typename boost::variant<int, std::string> variant_t;
  typedef typename cv_qualifier<variant_t>::type cv_variant_t;
  typedef typename boost::variant_alternative<0, cv_variant_t>::type cv_first_type;
  typedef typename boost::variant_alternative<1, cv_variant_t>::type cv_second_type;
  typedef typename boost::remove_cv<cv_first_type>::type first_type;
  typedef typename boost::remove_cv<cv_second_type>::type second_type;
  
  BOOST_CHECK(cv_check<cv_first_type>::value);
  BOOST_CHECK(cv_check<cv_second_type>::value);
  
  variant_t v;
  v = 1;
  BOOST_CHECK(boost::get<first_type>(v) == 1);
  v = "2";
  BOOST_CHECK(boost::get<second_type>(v) == "2");
}

void test_over_sequence_type() {
  typedef boost::variant<int> v1;
  typedef boost::variant<std::string> v2;
  typedef boost::make_variant_over<boost::mpl::joint_view<v1::types, v2::types>::type>::type variant_t;
  typedef typename boost::variant_alternative<0, variant_t>::type first_type;
  typedef typename boost::variant_alternative<1, variant_t>::type second_type;
  variant_t v;
  v = 1;
  BOOST_CHECK(boost::get<first_type>(v) == 1);
  v = "2";
  BOOST_CHECK(boost::get<second_type>(v) == "2");
}

int test_main(int , char* []) {
  test_correct_type();
  test_correct_cv<boost::add_const, boost::is_const>();
  test_correct_cv<boost::add_volatile, boost::is_volatile>();
  test_correct_cv<boost::add_cv, is_cv>();
  test_over_sequence_type();
  test_correct_alias_type();
  return 0;
}