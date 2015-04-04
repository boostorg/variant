
//-----------------------------------------------------------------------------
// boost-libs variant/test/lambda_test1.cpp source file
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

   typedef variant< int, string, double > t_var1;


   t_var1 v1;



   //
   // Check the standard call, returning nothing
   //
   double v1a = 0.;
   char   v1b = '0';
   string v1c = "";

   auto ts = boost::make_lambda_visitor(
		   [&](string & st){v1c = st;},
   	   	   [&](int c)	  {v1b = c;},
		   [&](const double & db) {v1a = db;}
   	   	   );

   static_assert(boost::is_same<typename decltype(ts)::return_type, void>::value, "Return type is incorrect");


   v1 = 5.9;
   apply_visitor(ts, v1);

   BOOST_CHECK(v1a == 5.9);
   BOOST_CHECK(v1b == '0');
   BOOST_CHECK(v1c == "");


   v1 = "Yippie-Ki-Yay";
   apply_visitor(ts, v1);

   BOOST_CHECK(v1a == 5.9);
   BOOST_CHECK(v1b == '0');
   BOOST_CHECK(v1c == "Yippie-Ki-Yay");


   v1 = 'C';
   apply_visitor(ts, v1);

   BOOST_CHECK(v1a == 5.9);
   BOOST_CHECK(v1b == 'C');
   BOOST_CHECK(v1c == "Yippie-Ki-Yay");


   ///ok, now with a return value
   auto ts2 = boost::make_lambda_visitor(
		   [&](string  st)  -> string {return st;},
   	   	   [&](int i)	   -> string  {return std::to_string(i);},
		   [&](double  db)  -> string {return std::to_string(db);}
   	   	   );

   static_assert(boost::is_same<typename decltype(ts2)::return_type, string>::value, "Return type is incorrect");

   v1 = 3.124;
   BOOST_CHECK(apply_visitor(ts2, v1) == std::to_string(3.124));


   v1 = "C3P0";
   BOOST_CHECK(apply_visitor(ts2, v1) == "C3P0");


   v1 = 42;
   BOOST_CHECK(apply_visitor(ts2, v1) == std::to_string(42));


}



int test_main(int , char* [])
{
   run();
   return 0;
}
