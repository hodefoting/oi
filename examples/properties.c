#include <stdio.h>
#include "oi.h"

static void cb (const char *name, void *value, void* oi)
{
  printf ("%s %f\n", name, oi@oi:get_float(name));
}
void list_properties (Var oi)
{
  oi@property:each (cb, oi);
}

@main ()
{
  Var test = var_new(NULL,NULL);

  list_properties (test);
  test@["abc"float]=3.2;
  test@["times"string]="3.1415";
  list_properties (test);

  test@ref:dec();
  return 0;
}
