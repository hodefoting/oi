#include <stdio.h>
#include "oi.h"


typedef struct
{
  OiCapability capability;
  float   foo;
}  __attribute((packed))  Foo;

static void foo_init (Oi *oi, OiCapability *capability, Oi *args)
{
  Foo *foo = (Foo*)capability;
  foo->foo = 1;
}

/* this creates the capability FOO that is the public handle to our type
 */
OI(FOO, Foo, NULL, foo_init, NULL)

float oi_get_foo (Oi *oi)
{
  Foo *foo = (Foo*)oi_capability_ensure (oi, FOO, NULL);
  return foo->foo;
}

void  oi_set_foo (Oi *oi, float f)
{
  Foo *foo = (Foo*)oi_capability_ensure (oi, FOO, NULL);
  foo->foo = f;
  oi_message_emit (oi, "notify", "foo");
}

int main (int argc, char **argv)
{
  Oi *test;
  Oi *string;

  test = oi_new ();

  string = oi_string_new ("fjo\n");
  oi_string_append_str (string, "foo");
  oi_string_append_str (string, " bar");
  printf ("%s\n", oi_string_get (string));
  oi_set_foo (string, 1.2);
  printf ("%f\n", oi_get_foo (string));

  oi_set_float (string, "foo", 1.2);
  oi_unref (string);
  oi_unref (test);

  return 0;
}
