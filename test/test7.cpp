// File: test7.cpp

#include "boost/test/minimal.hpp"

#include "boost/config.hpp"
#include "boost/variant.hpp"
#include "jobs.h"
#include "varout.h"

#include <iostream>
#include <algorithm>
#include <string>


using namespace boost;
using namespace std;


struct jas
{
   jas(int n = 364) : n_(n) { }

   int n_;
};

bool operator==(const jas& a, const jas& b)
{
   return a.n_ == b.n_;
}



template<typename ValueType>
struct compare_helper : boost::static_visitor<bool>
{
   compare_helper(ValueType& expected) : expected_(expected) { }

   bool operator()(const ValueType& value)
   {
      return value == expected_;
   }

   template<typename T>
   bool operator()(const T& )
   {
      return false;         
   }

   ValueType& expected_;

};

template<typename VariantType, typename ExpectedType>
void var_compare(const VariantType& v, ExpectedType expected)
{
   compare_helper<ExpectedType> ch(expected);

   BOOST_CHECK(boost::apply_visitor(ch, v));
}


void run()
{   
   variant<short, string> v0;

   var_compare(v0, static_cast<short>(0));

   v0 = 8;
   var_compare(v0, static_cast<short>(8));

   v0 = "penny lane";
   var_compare(v0, string("penny lane"));

   variant<jas, string, int> v1, v2 = jas(195);
   var_compare(v1, jas(364));

   v1 = jas(500);
   v1.swap(v2);

   var_compare(v1, jas(195));
   var_compare(v2, jas(500));


   variant<string, int> v3;
   var_compare(v3, string(""));


}


int test_main(int , char* [])
{
   run();
   return 0;
}

