#include "format.h"

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::to_string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  long min = seconds / 60;
  long hour = min / 60;
  min = min % 60;
  long se = seconds % 60;
  string h, m, s;
  string output;
  h = to_string(hour);
  m = to_string(min);
  s = to_string(se);
  if (hour < 10) {
    h = "0" + h;
  } else if (hour == 0) {
    h = "00";
  }
  output = h + ":";
  if (min < 10) {
    m = "0" + m;
  } else if (min == 0) {
    m = "00";
  }
  output = output + m + ":";
  if (se < 10) {
    s = "0" + s;
  } else if (se == 0) {
    s = "00";
  }
  output = output + s;
  return output;
}