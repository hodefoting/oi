#include <stdio.h>
#include "oi.h"

@trait Foo
{
  float  number;
};

static void hello (const char *data, const char *userdata)
{
  printf ("hello %s\n", data?data:"");
  if (userdata)
    printf ("%s\n", userdata);
}

static void init ()
{
  foo->number = 1.0;
  foo = self@oi:trait_ensure (MESSAGE, NULL);
  self@message:listen((void*)self,(void*)foo, "hello", (void*)foo_hello, "hoi");
}

float get_it ()
{
  Foo *foo = self@oi:trait_ensure (FOO, NULL);
  return foo->number;
}

void  set_it (float f)
{
  Foo *foo = self@oi:trait_ensure (FOO, NULL);
  foo->number= f;
  self@message:emit ("notify", "foo");
}

@end

#define Var Oi

int main (int argc, char **argv)
{
  Var *test = @oi:new();

  test@foo:set_it (0.2);
  test@message:emit("hello", "world");

  test@ref:dec();

  return 0;
}
