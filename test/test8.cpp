//
// Boost.Test
//
#include <boost/test/minimal.hpp>

#include <boost/variant.hpp> 
#include <boost/extract.hpp>
#include <boost/static_visitor.hpp>
#include <boost/apply_visitor.hpp>

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace boost;

typedef variant<float, std::string, int, std::vector<std::string> > t_var1;

struct int_sum : static_visitor<void>
{
   int_sum() : result_(0) { }

   void operator()(int t) 
   {
      result_ += t;
   }

   void operator()(float ) { }
   void operator()(const std::string& ) { }
   void operator()(const std::vector<std::string>& ) { }

   int result_;
}; 

int test_main(int , char* [])
{
   int_sum acc;

   t_var1 v1 = 19;

   t_var1* pv = &v1;
   const t_var1* read_only_p = &v1;

   int* ptr = extract<int>(pv);

   int* p1 = extract<int>(read_only_p);
   BOOST_CHECK(p1 == 0);

   const int* cpf = extract<const int>(read_only_p);
   BOOST_CHECK(cpf == ptr);

   p1 = extract<int>(pv);
   const float* p2 = extract<float>(pv);
   const short* p3 = extract<short>(pv);

   BOOST_CHECK(p1 == ptr);
   BOOST_CHECK(p2 == 0);
   BOOST_CHECK(p3 == 0);


   BOOST_CHECK(*p1 == 19);

   apply_visitor(acc, v1);
   BOOST_CHECK(acc.result_ == 19);

   *p1 = 90;
   apply_visitor(acc, v1);
   BOOST_CHECK(acc.result_ == 109);

//
//
//
   v1 = 800;
   int& r1 = extract<int>(v1);
   BOOST_CHECK(r1 == 800);

   int count = 0;
   try
   {
      float& r2 = extract<float>(v1);
   }
   catch(boost::bad_extract& e)
   {
      ++count;
   }

   try
   {
      short& r3 = extract<short>(v1);
   }
   catch(boost::bad_extract& e)
   {
      ++count;
   }

   BOOST_CHECK(count == 2);
   
   apply_visitor(acc, v1);
   BOOST_CHECK(acc.result_ == 909);

   r1 = 920;
   apply_visitor(acc, v1);
   BOOST_CHECK(acc.result_ == 1829);


   const t_var1& c = v1;

   try
   {
      const int& dcr1 = extract<const int>(c);
      BOOST_CHECK(&dcr1 == ptr);
      BOOST_CHECK(dcr1 == 920);
   }
   catch(boost::bad_extract& e)
   {
      BOOST_CHECK(false);
   }

   try
   {
      const float& dcr2 = extract<const float>(c);
   }
   catch(boost::bad_extract& e)
   {
      ++count;
   }

   try
   {
      const short& dcr3 = extract<const short>(c);
   }
   catch(boost::bad_extract& e)
   {
      ++count;
   }

   BOOST_CHECK(count == 4);

       
   return 0;
}


