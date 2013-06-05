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

#ifndef _OI_H
#define _OI_H
#include <string.h>

/* The variadic intance with trait's we use for all out oi-pointers */
typedef struct _Trait * var;

/* definition of a trait-type - to be associated with an instance */
typedef struct 
{
  const char     *name;  /* for debugging purposes */
  int             size;  /* size of a trait instance */

  /* function initializing a trait instance (or NULL) */
  void          (*init)    (var self, void *trait, var args);

  /* additional intialization (used internally by oi) */
  void          (*init_int)(var self, void *trait);

  /* release resources held by trait instance */
  void          (*destroy) (var self, void *trait);
} Type;

/* this macro is used for creating the instance from the set of pointers,
 * as well as declaring a pointer to the instance.
 *
 */
#define OI(NAME, s, init, init_int, destroy) \
static Type NAME##_trait = {"" #NAME, sizeof (s), init, init_int, destroy};\
Type *NAME = &NAME##_trait;

/* create a new bare bone oi instance, 
 * type and args can be NULL to create an instance
 * without any initial trait.
 */
var      var_new      (Type *trait, void *trait_arg);

/* destroy a variable instance, @ref:dec should be
 * used instead; it doesn't even add the reference
 * trait if no references have been added.
 */
void     var_finalize (var oi);

#include "oi-mem.h"

#include "own.h"
/* core capabilities */
#include "trait.h"
#include "property.h"
#include "message.h"
#include "ref.h"
#include "mutex.h"
#include "list.h"
#include "oistring.h"
#include "program.h"

#endif
