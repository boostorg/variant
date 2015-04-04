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
constexpr T& forward( typename remove_reference<T>::type& t )
{
	return t;
}
template< class T >
constexpr T&& forward( typename remove_reference<T>::type&& t )
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
template<typename lambda, typename Ret, typename Arg0>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0) const>
{
	typedef Arg0 type0;
	static constexpr size_t arg_cout = 1;
	typedef Ret return_type;
};

template<typename lambda, typename Ret, typename Arg0, typename Arg1>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0, Arg1) const>
{
	typedef Arg0 type0;
	typedef Arg1 type1;

	static constexpr size_t arg_cout = 2;
	typedef Ret return_type;
};

template<typename lambda, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
										typename Arg5, typename Arg6>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const>
{
	typedef Arg0 type0;
	typedef Arg1 type1;
	typedef Arg2 type2;
	typedef Arg3 type3;
	typedef Arg4 type4;
	typedef Arg5 type5;
	typedef Arg6 type6;
	static constexpr size_t arg_cout = 7;
	typedef Ret return_type;
};

template<typename lambda, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
										typename Arg5, typename Arg6, typename Arg7>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const>
{
	typedef Arg0 type0;
	typedef Arg1 type1;
	typedef Arg2 type2;
	typedef Arg3 type3;
	typedef Arg4 type4;
	typedef Arg5 type5;
	typedef Arg6 type6;
	typedef Arg7 type7;

	static constexpr size_t arg_cout = 8;
	typedef Ret return_type;
};

template<typename lambda, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
										typename Arg5, typename Arg6, typename Arg7, typename Arg8>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const>
{
	typedef Arg0 type0;
	typedef Arg1 type1;
	typedef Arg2 type2;
	typedef Arg3 type3;
	typedef Arg4 type4;
	typedef Arg5 type5;
	typedef Arg6 type6;
	typedef Arg7 type7;
	typedef Arg8 type8;

	static constexpr size_t arg_cout = 9;
	typedef Ret return_type;
};

template<typename lambda, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
										typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
struct lambda_parameter_deduction<Ret(lambda::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9) const>
{
	typedef Arg0 type0;
	typedef Arg1 type1;
	typedef Arg2 type2;
	typedef Arg3 type3;
	typedef Arg4 type4;
	typedef Arg5 type5;
	typedef Arg6 type6;
	typedef Arg7 type7;
	typedef Arg8 type8;
	typedef Arg9 type9;

	static constexpr size_t arg_cout = 10;
	typedef Ret return_type;
};




//////////////////////////////////////////////////////////////////////////////////////////////////////////
// used for the actual deduction
//
template<typename Lambda>
struct lambda_deduct_parameter
{
	using types		  = typename lambda_parameter_deduction<decltype(&Lambda::operator())>::types;
	using return_type = typename lambda_parameter_deduction<decltype(&Lambda::operator())>::return_type;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// the void helper is used to allow a possible handling of a void lambda, by using a specialization
//
template<typename Return_Type, typename Lambda>
class lambda_void_helper
{
	Lambda _lambda;
public:
	template<size_t Index, typename ...Args>
	using arg = lambda_arg_type<Index, Args>;

	template<typename L>
	lambda_void_helper(L &&lambda) : _lambda(forward<L>(lambda)) {};
	using types			= typename lambda_deduct_parameter<Lambda>::types;
	using return_type	= typename lambda_deduct_parameter<Lambda>::return_type;

	return_type operator()(arg<0, types>) const
	{
		return _lambda(forward<Args>(args)...);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  used if the lambda returns void.
//
template<typename Lambda>
class lambda_void_helper<void, Lambda>
{
	Lambda _lambda;
public:
	template<typename L>
	lambda_void_helper(L &&lambda) : _lambda(forward<L>(lambda)) {};
	using types 		= typename lambda_deduct_parameter<Lambda>::types;
	using return_type	= typename lambda_deduct_parameter<Lambda>::return_type;


	template<typename ... Args>
	typename enable_if<lambda_fit_types<types, Args...>::value>::type operator()(Args&&... args) const
	{
		_lambda(forward<Args>(args)...);
	}
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
class lambda_visitor_helper<Lambda> : public lambda_void_helper<typename lambda_deduct_parameter<Lambda>::return_type, Lambda>
{
	using father = lambda_void_helper<typename lambda_deduct_parameter<Lambda>::return_type, Lambda>;
public:
	using return_type = typename father::return_type;
	using father::operator();

	template<typename L>
	lambda_visitor_helper(L &&lambda) : father(forward<L>(lambda)) {};
};

template<typename Lambda, typename ... Args>
class lambda_visitor_helper<Lambda, Args...> : public lambda_visitor_helper<Args...>, public lambda_void_helper<typename lambda_deduct_parameter<Lambda>::return_type, Lambda>
{
	using father = lambda_void_helper<typename lambda_deduct_parameter<Lambda>::return_type, Lambda>;
	static_assert(boost::is_same<typename father::return_type, typename lambda_visitor_helper<Args...>::return_type>::value, "return types must have the same type");
public:
	using lambda_visitor_helper<Args...>::operator();
	using return_type = typename father::return_type;
	using father::operator();

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
