
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

   typedef variant< int, double > t_var1;
   typedef variant<int, double, unsigned, char> t_var2;


   t_var1 v1;

   //test the function for void and not void

   double d = 0;
   v1 = 3.124;

   boost::apply_lambdas(v1, [&](double din) {d = din;}, [&](int in){});
   BOOST_CHECK(d == 3.124);

   v1 = 42;
   int i = boost::apply_lambdas(v1, [&](double din)->int {return din;}, [&](int in){return in;});
   BOOST_CHECK(i == 42);


   //check the generic lambda

    auto gen_vis = make_lambda_visitor<string>(
 		   	   [](auto val){return to_string(val);});

    t_var2 v2;

    v2 = 'a';
    BOOST_CHECK(apply_visitor(gen_vis, v2) == to_string('a'));

    v2 = 42u;
    BOOST_CHECK(apply_visitor(gen_vis, v2) == to_string(42u));

    v2 = 3.12;
    BOOST_CHECK(apply_visitor(gen_vis, v2) == to_string(3.12));

    v2 = -10;
    BOOST_CHECK(apply_visitor(gen_vis, v2) == to_string(-10));


    //look if the generic lambda is overloadable

    v2 = 42u;

    bool called_auto = false;
    bool called_unsigned = false;
    apply_lambdas<void>(v2, [&](auto v2){called_auto = true;}, [&](unsigned u){called_unsigned = true;});

    BOOST_CHECK(!called_auto);
    BOOST_CHECK(called_unsigned);


    enum tp
    {
 	   is_auto, is_double
    };

    v2 = 3.4;

    BOOST_CHECK(apply_lambdas<tp>(v2, [](double ){return is_double;}, [](auto) {return is_auto;}) == is_double);


    v2 = 42;

    BOOST_CHECK(apply_lambdas<tp>(v2, [](double ){return is_double;}, [](auto) {return is_auto;}) == is_auto);

}




