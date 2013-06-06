#include <stdio.h>
#include "oi.h"

@trait Foo
{
};

"hello" (const char *data, Foo *foo)
{
  printf ("hello %s\n", data?data:"(NULL)");
  return 0;
}

@end

@main ()
{
  var test = var_new(FOO, NULL);
  test@"hello"("void");
  test@"hello"(NULL);
  test@ref:dec();
  self@ref:dec();
  return 0;
}
