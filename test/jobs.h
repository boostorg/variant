

#ifndef _JOBSH_INC_
#define _JOBSH_INC_


#include <vector>
#include <sstream>
#include <algorithm>
#include <typeinfo>
#include <boost/variant.hpp>

struct total_sizeof 
{
   typedef int return_type;
   
   total_sizeof() : total_(0) { }

   template<class Value>
   int operator()(const Value& value) const
   {
      total_ += sizeof(value);
      return total_;
   }

   int result() const
   {
      return total_;
   }

   mutable int total_;

}; // total_sizeof



//Function object: sum_int
//Description: Compute total sum of a series of numbers, (when called successively)
//Use sizeof(T) if applied with a non-integral type
struct sum_int
{
   typedef int return_type;
   
   sum_int() : total_(0) { }


   template<int n>
   struct int_to_type
   {
      BOOST_STATIC_CONSTANT(int, value = n);
   }; 

   //Integral type - add numerical value
   template<typename T>
   void add(T t, int_to_type<true> ) const
   {
      total_ += t;
   }

   //Other types - add sizeof<T>
   template<typename T>
   void add(T& , int_to_type<false> ) const
   {
      total_ += sizeof(T);
   }

   template<typename T>
   int operator()(const T& t) const
   {
      //Int_to_type is used to select the correct add() overload
      add(t, int_to_type<boost::is_integral<T>::value>());
      return total_;
   }

   int result() const
   {
      return total_;
   }

private:
   mutable int total_;

}; //sum_int






//Function object: sum_double
//Description: Compute total sum of a series of numbers, (when called successively)
//Accpetable input types: float, double (Other types are silently ignored)
struct sum_double
{
   typedef void return_type;
   
   sum_double() : total_(0) { }

   void operator()(float value) const
   {
      total_ += value;
   }

   void operator()(double value) const
   {
      total_ += value;
   }

   template<typename T>
   void operator()(const T&) const
   {
      //Do nothing
   }

   double result() const
   {
      return total_;
   }

private:
   mutable double total_;

}; //sum_double



struct int_printer
{
   typedef std::string return_type;
   
   int_printer(std::string prefix_s = "") : prefix_s_(prefix_s) { }
   int_printer(const int_printer& other) : prefix_s_(other.prefix_s_)
   {
      ost_ << other.str();
   }

   return_type operator()(int x) const
   {
      ost_ << prefix_s_ << x;
      return str();
   }

   return_type operator()(const std::vector<int>& x) const
   {
      ost_ << prefix_s_;

      //Use another Int_printer object for printing a list of all integers
      int_printer job(",");
      ost_ << std::for_each(x.begin(), x.end(), job).str();
      
      return str();
   }

   std::string str() const
   {
      return ost_.str();
   }

private:
   std::string prefix_s_;
   mutable std::ostringstream ost_;
};  //int_printer


struct int_adder
{
   typedef void return_type;
   
   int_adder(int rhs) : rhs_(rhs) { }

   void operator()(int& lhs) const
   {
      lhs += rhs_;
   }

   template<typename T>
   void operator()(const T& ) const
   {
      //Do nothing
   }

   int rhs_;
}; //int_adder




struct held_type_name
{
   typedef std::string return_type;
   
   template<typename T>
   std::string operator()(const T& ) const
   {
      ost_ << '[' << typeid(T).name() << ']';
      return result();
   }

   std::string result() const
   {
      return ost_.str();
   }

   mutable std::ostringstream ost_;

}; //held_type_name




template<typename T>
struct spec 
{
   typedef T result;
};

template<typename VariantType, typename S>
void verify(const VariantType& v, spec<S>, std::string str = "")
{
   using namespace boost;

   BOOST_CHECK(boost::apply_visitor(total_sizeof(), v) == sizeof(S));
   BOOST_CHECK(v.type() == typeid(S));

   //TODO: Check variant_cast : reference + point

   if(str.length() > 0)
   {
      std::string temp = boost::apply_visitor(to_text(), v);
      BOOST_CHECK(temp == str);         
   }
}


template<typename VariantType, typename S>
void verify_not(const VariantType& v, spec<S>)
{
   BOOST_CHECK(v.type() != typeid(S));
   //TODO: Check variant_cast : reference + point
}


#endif //_JOBSH_INC_
