#include <stdio.h>
#include "oi.h"

static void each_arg_cb (var argb, void *oi)
{
  printf ("  %s\n", argb@string:get());
}

@main ()
{
  var test = var_new (NULL,NULL);

  printf ("%s\n", self@["name"oi]@string:get());
  printf ("%i arguments\n", args@list:get_size());

  args@list:each ((void*)each_arg_cb, NULL);

  test@ref:dec();
  return 0;
}
