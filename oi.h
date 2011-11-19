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
typedef struct OiCapability
{
  OiType *type;
} OiCapability;

struct _OiType
{
  const char     *name;  /* for debugging purposes */
  struct _OiType *parent;/* parent capability, or NULL (currently NULL and unused)*/
  int             size;  /* size of a capability instance */

  /* function initializing a capability instance (or NULL) */
  void          (*init)    (Oi *oi, OiCapability *capability, Oi *args);
  /* function destroying a capability instance (or NULL)*/
  void          (*destroy) (Oi *oi, OiCapability *capability);
};

/* convenience for implementing capabilities, sticking this in a C file creates
 * the capability.
 *
 * it is enough to have   extern OiType  *STRING;
 */
#define OI(NAME, s, parent, init, destroy) \
static OiType NAME##_class = {"" #NAME, parent, sizeof (s), init, destroy};\
OiType *NAME = &NAME##_class;

/* create a new bare bone oi instance */
Oi           *oi_new                   (void);

/* adds an capability to an instance */
void          oi_capability_add        (Oi *oi, OiType *capability, Oi *args);

/* remove a capability from an instance */
void          oi_capability_remove     (Oi *oi, OiType *capability);

/* checks if the object has the given instance */
int           oi_capability_check        (Oi *oi, OiType *capability);

/* gets the capability, if any */
OiCapability *oi_capability_get        (Oi *oi, OiType *capability);

/* gets an capability, if capability doesn't already exist fail with warning 
 * (and segfault) */
OiCapability *oi_capability_get_assert (Oi *oi, OiType *capability);

/* gets the capability, creates and adds it if it doesn't already exist */
OiCapability *oi_capability_ensure     (Oi *oi, OiType *capability, Oi *args);

/* get a list of capabilities, the returned list of pointers is NULL terminated
 * and should not be freed by the caller.
 */
const OiCapability **oi_capability_list     (Oi *oi, int *count);

/* destroy an oi instance (normally not used, use oi_ref and oi_unref instead)*/
void          oi_destroy               (Oi *oi);


#include "oi-mem.h"

/* core capabilities */
#include "message.h"
#include "refcount.h"
#include "lock.h"
#include "properties.h"
#include "list.h"
#include "oistring.h"

#endif
