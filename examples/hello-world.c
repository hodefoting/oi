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

float get_it ()
{
  Foo *foo = self@oi:capability_ensure (FOO, NULL);
  return foo->number;
}

void  set_it (float f)
{
  Foo *foo = self@oi:capability_ensure (FOO, NULL);
  foo->number= f;
  self@message:emit ("notify", "foo");
}

@end

int main (int argc, char **argv)
{
  Oi *test;
  Oi *string;

  test = oi_new ();

  string = string_new ("fjo\n");
  string@string:append_str ("foo");
  string@string:append_str (" bar");

  printf ("%s\n", string@string:get ());
  string@foo:set_it (1.2);
  printf ("%f\n", string@foo:get_it());

  string@["boo"float]=1.4;
  printf ("::%f\n", string@oi:get_float("boo"));
  string@oi:unref ();
  test@oi:unref ();

  return 0;
}
