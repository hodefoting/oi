#include <stdio.h>
#include "oi.h"

static void each_arg_cb (var argb, void *oi)
{
  printf ("  %s\n", argb@string:get());
}

@main ()
{
  var test = var_new (NULL,NULL);

  printf ("%s\n", self@["name"string]);
  printf ("%i arguments\n", args@list:get_size());

  args@list:each ((void*)each_arg_cb, NULL);

  test@ref:dec();

  self@ref:dec(); /* needed to satisfy valgrind */
  return 0;
}
