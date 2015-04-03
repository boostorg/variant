
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
#include "boost/variant/multivisitors.hpp"

#include "class_a.h"
#include "jobs.h"

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

   typedef variant< int, double > t_var;


   t_var v1, v2, v3, v4;



   enum types_t
   {
	   iiii,
	   iiid,
	   iidi,
	   iidd,
	   idii,
	   idid,
	   iddi,
	   iddd,
	   diii,
	   diid,
	   didi,
	   didd,
	   ddii,
	   ddid,
	   dddi,
	   dddd,
   };

   //
   //Very basic, btu if the thing works, i would consider my the lambda visitor to work.
   //

   using dbl = double;

   auto ts = boost::make_lambda_visitor(
		   [](int, int, int, int)	{return iiii;},
		   [](int, int, int, dbl)	{return iiid;},
		   [](int, int, dbl, int)	{return iidi;},
		   [](int, int, dbl, dbl)	{return iidd;},
		   [](int, dbl, int, int)	{return idii;},
		   [](int, dbl, int, dbl)	{return idid;},
		   [](int, dbl, dbl, int)	{return iddi;},
		   [](int, dbl, dbl, dbl)	{return iddd;},
		   [](dbl, int, int, int)	{return diii;},
		   [](dbl, int, int, dbl)	{return diid;},
		   [](dbl, int, dbl, int)	{return didi;},
		   [](dbl, int, dbl, dbl)	{return didd;},
		   [](dbl, dbl, int, int)	{return ddii;},
		   [](dbl, dbl, int, dbl)	{return ddid;},
		   [](dbl, dbl, dbl, int)	{return dddi;},
		   [](dbl, dbl, dbl, dbl)	{return dddd;});

   static_assert(boost::is_same<typename decltype(ts)::return_type, types_t>::value, "Return type is incorrect");


   v1 = 0 ; v2 = 0 ; v3 = 0 ; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iiii);
   v1 = 0 ; v2 = 0 ; v3 = 0 ; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iiid);
   v1 = 0 ; v2 = 0 ; v3 = 0.; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iidi);
   v1 = 0 ; v2 = 0 ; v3 = 0.; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iidd);
   v1 = 0 ; v2 = 0.; v3 = 0 ; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == idii);
   v1 = 0 ; v2 = 0.; v3 = 0 ; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == idid);
   v1 = 0 ; v2 = 0.; v3 = 0.; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iddi);
   v1 = 0 ; v2 = 0.; v3 = 0.; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == iddd);
   v1 = 0.; v2 = 0 ; v3 = 0 ; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == diii);
   v1 = 0.; v2 = 0 ; v3 = 0 ; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == diid);
   v1 = 0.; v2 = 0 ; v3 = 0.; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == didi);
   v1 = 0.; v2 = 0 ; v3 = 0.; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == didd);
   v1 = 0.; v2 = 0.; v3 = 0 ; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == ddii);
   v1 = 0.; v2 = 0.; v3 = 0 ; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == ddid);
   v1 = 0.; v2 = 0.; v3 = 0.; v4 = 0 ;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == dddi);
   v1 = 0.; v2 = 0.; v3 = 0.; v4 = 0.;    BOOST_CHECK(apply_visitor(ts, v1, v2, v3, v3) == dddd);

}



int test_main(int , char* [])
{
   run();
   return 0;
}
