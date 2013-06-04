#include <stdio.h>
#include "oi.h"

@main ()
{
  Var *test = string_new("test");

  printf ("%s\n", test@string:get());
  test = test@string:append_printf(" %i", 23);
  printf ("%s\n", test@string:get());
    
  return 0;
}
