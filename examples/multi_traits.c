#include <stdio.h>
#include "oi.h"
@trait Foo
{
  float  number;
};
static void init ()
{
  foo->number = 42.0;
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
  float  number;
};
static void init ()
{
  bar->number = 23.0;
}
void  set_it (float f)
{
  Bar *bar = self@trait:ensure (BAR, NULL);
  bar->number= f;
}
float get_it ()
{
  Bar *foo = self@trait:ensure (BAR, NULL);
  return foo->number;
}
@end

@main ()
{
  var test;

  test = var_new (NULL, NULL);

  printf ("%f\n", test@foo:get_it());
  test@foo:set_it (1.2);
  printf ("%f\n", test@foo:get_it());

  printf ("%f %f\n", test@foo:get_it(), test@bar:get_it());
  test@bar:set_it (1.4);
  printf ("%f %f\n", test@foo:get_it(), test@bar:get_it());

  test@trait:remove(FOO);
  printf ("%f %f\n", test@foo:get_it(), test@bar:get_it());

  test@ref:dec ();
  return 0;
}
