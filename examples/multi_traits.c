#include <stdio.h>
#include "oi.h"
@trait Foo
{
  float  number;
};
static void init ()
{
  this->number = 42.0;
}
void  set_it (float f)
{
  this->number= f;
}
float get_it ()
{
  return this->number;
}
@end

@trait Bar
{
  float  number;
};
static void init ()
{
  this->number = 23.0;
}
void  set_it (float f)
{
  this->number= f;
}
float get_it ()
{
  return this->number;
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
  self@ref:dec ();
  return 0;
}
