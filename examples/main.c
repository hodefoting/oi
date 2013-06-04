#include <stdio.h>
#include "oi.h"

static void cb (const char *name, void *value, void* oi)
{
  printf ("%s %f\n", name, oi@oi:get_float(name));
}

static void each_arg_cb (Var *argb, void *oi)
{
  printf ("  %s\n", argb@string:get());
}

@main ()
{
  Var *test = @var:new();

  test@oi:properties_each (cb, self);
  test@["abc"float]=3.2;
  test@["times"string]="3.1415";
  test@oi:properties_each (cb, self);

  printf ("%s\n", self@["name"oi]@string:get());

  args@list:each ((void*)each_arg_cb, self);

  test@ref:dec();
  return 0;
}
