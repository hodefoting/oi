#include <stdio.h>
#include "oi.h"
@trait Foo
{
  float  number;
};

static void init ()
{
  this->number = 1.0;
}

void  set_it (float f)
{
  Foo *this = self@trait:ensure (FOO, NULL);
  this->number= f;
}

float get_it ()
{
  Foo *this = self@trait:ensure (FOO, NULL);
  return this->number;
}

@end

@main ()
{
  var test;

  test = var_new(NULL, NULL);

  printf ("%f\n", test@foo:get_it());
  test@foo:set_it (1.2);
  printf ("%f\n", test@foo:get_it());

  test@ref:dec ();

  return 0;
}
