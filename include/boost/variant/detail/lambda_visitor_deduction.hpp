//  Boost.Varaint
//  Lambda Visitor utilities defined here
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Klemens Morgenstern, 2015.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#ifndef BOOST_VARIANT_DETAIL_LAMBDA_VISITOR_DEDUCTION_HPP
#define BOOST_VARIANT_DETAIL_LAMBDA_VISITOR_DEDUCTION_HPP

#include <boost/type_traits/is_same.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace boost {
namespace detail { namespace variant {

//cause i don't wanna include utility; doesn't really make sense, since i already use c++14
template< class T >
constexpr T&& forward( typename remove_reference<T>::type& t ) noexcept
{
	return static_cast<T&&>(t);
}
template< class T >
constexpr T&& forward( typename remove_reference<T>::type&& t )noexcept
{
	return static_cast<T&&>(t);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// I did not find a SFINAE solution for multi-visitors, so I will do that by hand. up to 10 we go.
//


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// lambda_parameter_deduction deduct the return type and the parameters of the input.
//
//
template<typename lambda>
struct lambda_parameter_deduction
{
};
//the ... comes from possible mutli visitors.
template<typename lambda, typename Ret, typename ...Args>
struct lambda_parameter_deduction<Ret(lambda::*)(Args...) const>
{
//	typedef Arg0 type0;
	static constexpr size_t arg_cout = sizeof...(Args);
	typedef Ret return_type;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// used for the actual deduction
//
template<typename Lambda>
struct lambda_deduct_parameter
{
	//using types		  = typename lambda_parameter_deduction<decltype(&Lambda::operator())>::types;
	using return_type = typename lambda_parameter_deduction<decltype(&Lambda::operator())>::return_type;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  declaration of the actual visitor, i.e. this one is partial and gets inherited
//
template<typename ... Args>
class lambda_visitor_helper
{
	void operator()();
};

template<typename Lambda>
class lambda_visitor_helper<Lambda> : public Lambda
{
	using father = Lambda;
public:
	using return_type = typename lambda_deduct_parameter<Lambda>::return_type;
	using father::operator();

	template<typename L>
	lambda_visitor_helper(L &&lambda) : father(forward<L>(lambda)) {};
};

template<typename Lambda, typename ... Args>
class lambda_visitor_helper<Lambda, Args...> : public lambda_visitor_helper<Args...>, Lambda
{
	using father = Lambda;
public:
	using lambda_visitor_helper<Args...>::operator();
	using return_type = typename lambda_deduct_parameter<Lambda>::return_type;
	using father::operator();

	static_assert(boost::is_same<return_type, typename lambda_visitor_helper<Args...>::return_type>::value, "return types must have the same type");


	template<typename L, typename ... l_Args>
	lambda_visitor_helper(L && lambda, l_Args && ...ls) :
			lambda_visitor_helper<Args...>(forward<l_Args>(ls)...), father(forward<Lambda>(lambda)) {};
};

template<typename ...Args>
struct lambda_visitor : public lambda_visitor_helper<Args...>, public boost::static_visitor<typename lambda_visitor_helper<Args...>::return_type>
{
	using return_type = typename lambda_visitor_helper<Args...>::return_type;
	using lambda_visitor_helper<Args...>::operator();
	lambda_visitor(Args&&...args) : lambda_visitor_helper<Args...>(forward<Args>(args)...) {};
};


}}}




#endif /* BOOST_VARIANT_DETAIL_LAMBDA_VISITOR_DEDUCTION_HPP_ */
