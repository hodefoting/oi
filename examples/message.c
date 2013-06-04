#include <stdio.h>
#include "oi.h"
@trait Foo
{
};

"hello" (const char *data, Foo *foo)
{
  int i;
  for (i = 0; i < self@["times"int]; i++)
    printf ("hello %s\n", data?data:NULL);
}
@end

int main (int argc, char **argv)
{
  Oi *test = @oi:new();
  test@["times"int]=3;
  test@oi:trait_add(FOO, NULL);
  test@message:emit("hello", "world");
  //test@"hello"("world");
  test@ref:dec();
  return 0;
}
