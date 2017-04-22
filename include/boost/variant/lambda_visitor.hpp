//  Boost.Varaint
//  Multivisitors defined here
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Klemens Morgenstern, 2015.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#ifndef BOOST_VARIANT_LAMBDA_VISITOR_HPP
#define BOOST_VARIANT_LAMBDA_VISITOR_HPP

#include "boost/variant/static_visitor.hpp"
#include "boost/variant/detail/lambda_visitor_deduction.hpp"

namespace boost {



template<typename ...Lambdas>
detail::variant::lambda_visitor<Lambdas...> make_lambda_visitor(Lambdas && ...args) {return detail::variant::lambda_visitor<Lambdas...>(detail::variant::forward<Lambdas>(args)...);}


template<typename Variant, typename ...Lambdas>
auto apply_lambdas(Variant & var, Lambdas ... lambdas) -> typename detail::variant::lambda_visitor<Lambdas...>::return_type
{
	using Ret = typename detail::variant::lambda_visitor<Lambdas...>::return_type;
	auto vis = make_lambda_visitor(detail::variant::forward<Lambdas>(lambdas)...);
	return Ret(var.apply_visitor(vis));//type
};

///explicit version

template<typename ReturnType, typename ...Lambdas>
detail::variant::lambda_visitor_exp<ReturnType, Lambdas...> make_lambda_visitor(Lambdas && ...args) {return detail::variant::lambda_visitor_exp<ReturnType, Lambdas...>(detail::variant::forward<Lambdas>(args)...);}


template<typename ReturnType, typename Variant, typename ...Lambdas>
auto apply_lambdas(Variant & var, Lambdas ... lambdas) -> ReturnType
{
//	using Ret = typename detail::variant::lambda_visitor<Lambdas...>::return_type;
	auto vis = make_lambda_visitor<ReturnType>(detail::variant::forward<Lambdas>(lambdas)...);
	return ReturnType(var.apply_visitor(vis));//type
};



}



#endif /* LAMBDA_VISITOR_H_ */
