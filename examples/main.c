#include <stdio.h>
#include "oi.h"

static void each_arg_cb (Var argb, void *oi)
{
  printf ("  %s\n", argb@string:get());
}

@main ()
{
  Var test = var_new (NULL,NULL);

  test@property:each (cb, test);

  printf ("%s\n", self@["name"oi]@string:get());
  printf ("%i arguments\n", args@list:get_size());

  args@list:each ((void*)each_arg_cb, NULL);

  test@ref:dec();
  return 0;
}
