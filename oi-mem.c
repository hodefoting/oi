/* Copyright (c) 2011,2013 Øyvind Kolås
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Øyvind Kolås    <pippin@gimp.org>
 */

#include <stdlib.h>
#include <string.h>

#ifdef strdup
#undef strdup
#endif

typedef struct varMem {
  void  (*free)    (size_t size, void *mem);
  void *(*malloc)  (size_t size);
  void *(*realloc) (void *mem, size_t size);
  char *(*strdup)  (const char *str);
  void  (*strfree) (void *str);
} varMem;

static void oi_free_default (size_t size, void *mem)
{
  free (mem);
}

static varMem memfuncs = {oi_free_default, malloc, realloc, strdup, free};

void oi_set_memory_functions (
  void  (*free)    (size_t size, void *mem),
  void *(*malloc)  (size_t size),
  void *(*realloc) (void *mem, size_t size),
  char *(*strdup)  (const char *str),
  void  (*strfree) (void *str))
{
  if (free)    memfuncs.free    = free;
  if (malloc)  memfuncs.malloc  = malloc;
  if (realloc) memfuncs.realloc = realloc;
  if (strdup)  memfuncs.strdup  = strdup;
  if (strfree) memfuncs.strfree = strfree;
}

void oi_free (size_t size, void *mem)
{
  memfuncs.free (size, mem);
}

void *oi_malloc (size_t size)
{
  return memfuncs.malloc (size);
}
void *oi_realloc (void *mem, size_t size)
{
  return memfuncs.realloc (mem, size);
}
char *oi_strdup (const char *str)
{
  return memfuncs.strdup (str);
}
void oi_strfree (const char *mem)
{
  memfuncs.strfree ((void*)mem);
}
