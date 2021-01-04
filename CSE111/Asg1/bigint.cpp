// bigint.cpp by Erico Bayani and Venkat Vetsa
// <ebayani@ucsc.edu> <vvetsa@ucsc.edu> 

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): ubig_value(that), is_negative (that < 0) {
  //   DEBUGF ('~', this << " -> " << ubig_value)
}

bigint::bigint (const ubigint& ubig_value_, bool is_negative_):
                ubig_value(ubig_value_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   ubig_value = ubigint (that.substr (is_negative ? 1 : 0));
}

// arithmatic operators

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {ubig_value, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
  bigint result;
  if (is_negative == that.is_negative){
    
   result.ubig_value = ubig_value + that.ubig_value;
   result.is_negative = is_negative;
   
  }
  else {
    
    result.ubig_value = (ubig_value >= that.ubig_value) ?
      ubig_value - that.ubig_value : that.ubig_value - ubig_value;
    result.is_negative = (ubig_value >= that.ubig_value) ?
      is_negative : that.is_negative;
  }
  if (result.ubig_value == 0) result.is_negative = false;
   return result;
}

bigint bigint::operator- (const bigint& that) const {
  bigint result;
  if ((is_negative != that.is_negative)){

    result.ubig_value = (ubig_value >= that.ubig_value) ?
      ubig_value + that.ubig_value : that.ubig_value + ubig_value;
    result.is_negative = (ubig_value <= that.ubig_value) ?
      is_negative : that.is_negative;
   
  }
  else  {
    
   result.ubig_value = (ubig_value >= that.ubig_value) ?
     ubig_value - that.ubig_value : that.ubig_value - ubig_value;
   result.is_negative = (ubig_value <= that.ubig_value) ?
     !is_negative : that.is_negative;
   
   
  }

  if (result.ubig_value == 0) result.is_negative = false;
   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result;
   result.ubig_value = ubig_value * that.ubig_value;
   result.is_negative = (is_negative == that.is_negative) ?
     false : true;
   if (result.ubig_value == 0) result.is_negative = false;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result;
   result.ubig_value = ubig_value /that.ubig_value;
   result.is_negative = (is_negative == that.is_negative) ?
     false : true;
   if (result.ubig_value == 0) result.is_negative = false;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = ubig_value % that.ubig_value;
   if (result.ubig_value == 0) result.is_negative = false;
   return result;
}

// comparison operators 

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and
     ubig_value == that.ubig_value;
}

bool bigint::operator!= (const bigint& that) const {
  bool equal = (*this == that);
  return !equal;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? ubig_value > that.ubig_value
                      : ubig_value < that.ubig_value;
}

bool bigint::operator> (const bigint& that) const {
  return (*this == that) ? false : !(*this < that);
}



bool bigint::operator>= (const bigint& that) const {
  return (*this == that) ? true : !(*this < that);
}

bool bigint::operator<= (const bigint& that) const {
  return (*this == that) ? true : *this < that; 
}

// print operator

ostream& operator<< (ostream& out, const bigint& that) {
  return out << (that.is_negative ? "-" : std::string())
               << that.ubig_value;
}

