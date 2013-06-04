#include <stdio.h>
#include "oi.h"

@trait Foo
{
};

"hello" (const char *data, Foo *foo)
{
  printf ("hello %s\n", data?data:"(NULL)");
}

@

int main (int argc, char **argv)
{
  Oi *test = @oi:new();
  test@oi:trait_add(FOO, NULL);
  test@"hello"("void");
  test@"hello"(NULL);
  test@ref:dec();
  return 0;
}
