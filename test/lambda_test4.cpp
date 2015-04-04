
//-----------------------------------------------------------------------------
// boost-libs variant/test/lambda_test4.cpp source file
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


#include <iostream>
#include <string>
#include <vector>



void run4()
{

   using boost::apply_visitor;
   using boost::variant;
   using std::string;
   using std::vector;
   using std::cout;
   using std::endl;
   using std::nullptr_t;

   typedef variant< int, double > t_var;


   t_var v;




   //test the function for void and not void

   double d = 0;
   v = 3.124;

   boost::apply_lambdas(v, [&](double din) {d = din;}, [&](int in){});
   BOOST_CHECK(d == 3.124);

   v = 42;
   int i = boost::apply_lambdas(v, [&](double din)->int {return din;}, [&](int in){return in;});
   BOOST_CHECK(i == 42);



}




