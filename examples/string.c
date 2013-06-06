#include <stdio.h>
#include "oi.h"

@main ()
{
  var test = @"test";

  printf ("%s\n", test@string:get());
  test@string:append_printf(" %i", 23);
  printf ("%s\n", test@string:get());
  test@string:clear();
  test@string:append_str("hello ")@string:append_str("there");
  printf ("%s\n", test@string:get());

  test@ref:dec();
    
  self@ref:dec ();
  return 0;
}
