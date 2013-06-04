/* Copyright (c) 2011 Øyvind Kolås
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

#ifndef _OI_H
#define _OI_H
#include <string.h>

typedef struct _Trait Oi;  /* opaque handle to an object */
typedef struct _Type  Type;
struct _Type
{
  const char     *name;  /* for debugging purposes */
  int             size;  /* size of a trait instance */
  /* function initializing a trait instance (or NULL) */
  void          (*init)    (Oi *self, void *trait, Oi *args);
  void          (*init_int)(Oi *self, void *trait);
  /* function destroying a trait instance (or NULL)*/
  void          (*destroy) (Oi *self, void *trait);
};
#define OI(NAME, s, init, init_int, destroy) \
static Type NAME##_class = {"" #NAME, sizeof (s), init, init_int, destroy};\
Type *NAME = &NAME##_class;

/* create a new bare bone oi instance */
Oi     *oi_new      (void);
Oi     *oi_new_bare (Type *type, void *userdata);
void    oi_finalize (Oi *oi);

#include "oi-mem.h"
/* core capabilities */
#include "trait.h"
#include "properties.h"
#include "message.h"
#include "ref.h"
#include "mutex.h"
#include "list.h"

#include "oistring.h"
#include "program.h"

#endif
