#ifndef STRING_COMPARE_FUNCTOR_H
#define STRING_COMPARE_FUNCTOR_H

#include <string.h>

class StringCompareFunctor
{
 public:
  bool operator()(char const* a, char const* b)
  {
    return (strcmp(a, b) < 0);
  }
};

#endif
