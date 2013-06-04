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

typedef struct _Oi Oi;          /* opaque handle to an object */
typedef struct _OiType OiType;
typedef struct OiTrait
{
  OiType *type;
} OiTrait;

struct _OiType
{
  const char     *name;  /* for debugging purposes */
  int             size;  /* size of a trait instance */
  /* function initializing a trait instance (or NULL) */
  void          (*init)    (Oi *oi, OiTrait *trait, Oi *args);
  void          (*init_int)(Oi *oi, OiTrait *trait);
  /* function destroying a trait instance (or NULL)*/
  void          (*destroy) (Oi *oi, OiTrait *trait);
};

/* convenience for implementing capabilities, sticking this in a C file creates
 * the trait.
 *
 * it is enough to have   extern OiType  *STRING;
 */
#define OI(NAME, s, init, init_int, destroy) \
static OiType NAME##_class = {"" #NAME, sizeof (s), init, init_int, destroy};\
OiType *NAME = &NAME##_class;

/* create a new bare bone oi instance */
Oi             *oi_new                   (void);
void            oi_trait_add        (Oi *oi, OiType *trait, Oi *args);
void            oi_trait_remove     (Oi *oi, OiType *trait);
int             oi_trait_check      (Oi *oi, OiType *trait);
void           *oi_trait_get        (Oi *oi, OiType *trait);
void           *oi_trait_get_assert (Oi *oi, OiType *trait);
void           *oi_trait_ensure     (Oi *oi, OiType *trait, Oi *args);
const OiTrait **oi_trait_list       (Oi *oi, int *count);
void            oi_finalize         (Oi *oi);

Oi           *oi_new_bare (OiType *type, void *userdata);

Oi *oi_make_args (Oi *program, char **argv);

#include "oi-mem.h"

/* core capabilities */
#include "ref.h"
#include "message.h"
#include "mutex.h"
#include "properties.h"
#include "list.h"
#include "oistring.h"

#endif
