

//File: test4.cpp
//Written by: Itay Mamamn

//
//    IMPORTANT:
//
//RDSL sequence must be use for variants with 20 params. (Currently, MPL-based 
//variants support up to 8 params).
//
//Consequently, this test program will break MSVC7
//
//

//
// Boost.Test
//
#include <boost/test/minimal.hpp>


//
// Test program code starts here...
//


#include <boost/variant.hpp>

#include "common/varout.h"
#include "common/jobs.h"

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

using boost::apply_visitor;



template<typename U> 
struct x1
{
   U u_;
   U* u_p_;
}; 

template<typename U> 
struct x2
{
   int n_;
   const U* u_p_;
}; 

template<typename U> 
struct x3
{
   float f_;
   const U* u_p_;
}; 



template<typename T>
struct generator
{
   typedef x1<T> t1;
   typedef x2<T> t2;
   typedef x3<T> t3;

   typedef x1<T*> t4;
   typedef x2<T*> t5;
   typedef x3<T*> t6;

   typedef x1<t1> t7;
   typedef x2<t2> t8;
   typedef x3<t3> t9;

   typedef x1<t4> t10;
   typedef x2<t5> t11;
   typedef x3<t6> t12;

}; //generator

void run()
{
   using std::cout;
   using std::endl;
   using std::string;
   using boost::variant;
   using boost::incomplete;
   using boost::variant_cast;
   using boost::type_to_type;


   typedef variant<unsigned short, unsigned char, int> t_var1;
   typedef variant<string, float> t_var2;
   typedef variant<string, int> t_var3;

   t_var1 v1;
   t_var2 v2;
   t_var3 v3;

   const char c0 = 'x';
   v1 = c0;
   v2 = v1;
   v3 = v2;

   cout << "v1 = " << v1 << ", " << apply_visitor(held_type_name(), v1)<< endl;
   cout << "v2 = " << v2 << ", " << apply_visitor(held_type_name(), v2) << endl;

   assert(variant_cast(type_to_type<int>(), &v1) != 0);
   assert(variant_cast(type_to_type<float>(), &v2) != 0);
   assert(variant_cast(type_to_type<int>(), &v3) != 0);

   assert(apply_visitor(sum_int(), v1) == c0);
   assert(apply_visitor(sum_int(), v3) == c0); 


   //Check 'loaded' variants
   typedef generator<int> g1;
   typedef generator<double> g2;

   variant<
      g1::t1,  g1::t2,  g1::t3,  g1::t4, 
      g1::t5,  g1::t6,  g1::t7,  g1::t8, 
      g1::t9,  g1::t10, g1::t11, g1::t12, 
      g2::t1,  g2::t2,  g2::t3,  g2::t4, 
      g2::t5,  g2::t6,  g2::t7
   > big1;

   variant<
      g1::t5,  g1::t6,  g1::t7,  g1::t8, 
      g1::t9,  g1::t10, g1::t11, g1::t12, 
      g2::t1,  g2::t2,  g2::t3,  g2::t4, 
      g2::t5,  g2::t6,  g2::t7
   > big2;



   //test big1
   assert(big1.type() == typeid(g1::t1));
   assert(big2.type() == typeid(g1::t5));

   big1 = g1::t3();
   assert(big1.type() == typeid(g1::t3));

   big1 = big2;
   assert(big1.type() == typeid(g1::t5));

#ifndef BOOST_VARIANT_NO_ASSIGN_HELPER
   //
   // A variant instantiation with a single dot-style typelist parameter
   // is only supported if BOOST_VARIANT_NO_ASSIGN_HELPER is NOT defined
   //

   //test big3
   variant< BOOST_RDSL_22(
      g1::t1,  g1::t2,  g1::t3,  g1::t4, 
      g1::t5,  g1::t6,  g1::t7,  g1::t8, 
      g1::t9,  g1::t10, g1::t11, g1::t12, 
      g2::t1,  g2::t2,  g2::t3,  g2::t4, 
      g2::t5,  g2::t6,  g2::t7,  g2::t8,
      g2::t9,  g2::t10
   )> big3;
   assert(big3.type() ==typeid(g1::t1));

   big3 = g1::t5();
   assert(big3.type() == typeid(g1::t5));

   big3 = big2;
   assert(big3.type() == typeid(g1::t5));

#endif //BOOST_VARIANT_NO_ASSIGN_HELPER


}



int test_main(int , char* [])
{
   run();
   return 0;
}

