#include <stdio.h>
#include "oi.h"
@trait Foo
{
  float  number;
};

static void init ()
{
  foo->number = 1.0;
}

void  set_it (float f)
{
  Foo *foo = self@trait:ensure (FOO, NULL);
  foo->number= f;
}

float get_it ()
{
  Foo *foo = self@trait:ensure (FOO, NULL);
  return foo->number;
}

@end

@trait Bar
{
  int x;
  int y;
};

@end

@main ()
{
  Oi *test;

  test = oi_new ();

  printf ("%f\n", test@foo:get_it());
  test@foo:set_it (1.2);
  printf ("%f\n", test@foo:get_it());

  test@ref:dec ();

  return 0;
}
