
//-----------------------------------------------------------------------------
// boost-libs variant/test/lambda_test2.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2015
// Klemens Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/config.hpp"

#ifdef BOOST_MSVC
#pragma warning(disable:4244) // conversion from const int to const short
#endif

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"
#include "boost/variant/lambda_visitor.hpp"


#include <iostream>
#include <string>
#include <vector>



void run()
{

   using boost::apply_visitor;
   using boost::variant;
   using std::string;
   using std::vector;
   using std::cout;
   using std::endl;
   using std::nullptr_t;

   typedef variant< int, double > t_var1;
   typedef variant< string, nullptr_t> t_var2;

   t_var1 v1;
   t_var2 v2;


   enum types_t
   {
	   int_string,
	   double_string,
	   int_nullptr,
	   double_nullptr,
   };

   //
   // The whole test is done by the return value, cause that's simple.
   //


   auto ts = boost::make_lambda_visitor(
		   [](int, string )			{return int_string;},
   	   	   [](int, nullptr_t)		{return int_nullptr;},
		   [](double, string )		{return double_string;},
   	   	   [](double, nullptr_t)	{return double_nullptr;}

		    	   	   );

   static_assert(boost::is_same<typename decltype(ts)::return_type, types_t>::value, "Return type is incorrect");


   v1 = 5.9;
   v2 = nullptr;
   BOOST_CHECK(apply_visitor(ts, v1, v2) == double_nullptr);

   v1 = 42;
   BOOST_CHECK(apply_visitor(ts, v1, v2) == int_nullptr);

   v2 = "R2-D2";
   BOOST_CHECK(apply_visitor(ts, v1, v2) == int_string);

   v1 = 3.124;
   BOOST_CHECK(apply_visitor(ts, v1, v2) == double_string);
}



int test_main(int , char* [])
{
   run();
   return 0;
}
