/* Copyright (c) 2011, 2013 Øyvind Kolås
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

/* should probably a b-tree instead of a list to improve performance */

#include "oi.h"
#include "stdlib.h"
@generateheader

@trait Property 
{
  var props;
};

typedef enum
{
  OI_PTYPE_FLOAT,
  OI_PTYPE_INT,
  OI_PTYPE_STRING,
  OI_PTYPE_POINTER,
  OI_PTYPE_OI
} PropertyType;

typedef struct
{
  const char *name;
  PropertyType type;
  union {
    float       value_float;
    int         value_int;
    const char *value_string;
    void       *value_pointer;
    var          value_oi;
  } o;
} PropertyEntry;

static void prop_destroy! (PropertyEntry *entry, void *user_data);
static void init ()
{
  property->props = @list:new ();
  property->props@list:set_destroy ((void*) prop_destroy, NULL);
}

static void destroy ()
{
  property->props@var:finalize();
}

static void
each_wrapper! (void *item, void *user_data)
{
  void **args = user_data;
  PropertyEntry *entry = item;
  void (*cb)(const char *key, void *item, void *user_data) = args[0];
  cb (entry->name, "foo", args[1]);
}
/*
void each (void (*cb)(const char *key, void *item, void *user_data),
           void *user_data)
           */
void each (void *cb, void *user_data)
{
  void *args[] = {cb, user_data};
  Property *property = (self@trait:get (PROPERTY));
  if (property)
    property->props@list:each(each_wrapper, args);
}

static void prop_unset! (PropertyEntry *entry)
{
  if (entry->type == OI_PTYPE_STRING && entry->o.value_string)
    {
      oi_strfree ((void*)entry->o.value_string);
      entry->o.value_string = NULL;
    }
  else if (entry->type == OI_PTYPE_OI && entry->o.value_oi)
    {
      entry->o.value_oi@ref:dec ();
      entry->o.value_oi = NULL;
    }
}

static void prop_destroy! (PropertyEntry *entry, void *user_data)
{
  prop_unset (entry);
  if (entry->name)
    oi_strfree ((void*)entry->name);
  oi_free (sizeof (PropertyEntry), entry);
}

static int match_name! (void *property_entry, const char *name)
{
  PropertyEntry *entry = property_entry;
  return (entry && entry->name && name && !strcmp (entry->name, name));
}
static PropertyEntry *oi_get_entry_read! (var oi, const char *name)
{
  Property *property = ((void*)trait_ensure (oi, PROPERTY, NULL));
  int no;
  PropertyEntry *entry;
  no = property->props@list:find_custom ((void*)match_name, (void*)name);
  oi@mutex:lock (); /* XXX: is this lock really needed? */
  if (no >= 0)
    entry = property->props@list:get (no);
  else
    {
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup ();
      property->props@list:append (entry);
      entry->type = OI_PTYPE_INT;
      entry->o.value_int = 0;
    } 
  oi@mutex:unlock ();
  return entry;
}

static PropertyEntry *oi_get_entry_write! (var oi, const char *name)
{
  Property *property = ((void*)trait_ensure (oi, PROPERTY, NULL));
  int no;
  PropertyEntry *entry;
  oi@mutex:lock ();
  no = property->props@list:find_custom((void*)match_name, (void*)name);
  if (no >= 0)
    {
      entry = property->props@list:get(no);
      prop_unset (entry);
    }
  else
    {
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup();
      entry->o.value_int = 0;
      property->props@list:append (entry);
      entry->type = OI_PTYPE_INT;
    } /* XXX: oicc hack*/ ;
  oi@mutex:unlock ();
  return entry;
}

void   oi_set_float! (var oi, const char *name, float       value)
{
  PropertyEntry *entry = oi_get_entry_write (oi, name);
  int changed = entry->o.value_float != value;
  entry->o.value_float = value;
  entry->type = OI_PTYPE_FLOAT;
  if (changed)
    oi@"notify"((void*)name);
}

void   oi_set_int! (var oi, const char *name, int         value)
{
  PropertyEntry *entry = oi_get_entry_write (oi, name);
  int changed = entry->o.value_int != value;
  entry->type = OI_PTYPE_INT;
  entry->o.value_int = value;
  if (changed)
    oi@"notify"((void*)name);
}
void   oi_set_string! (var oi, const char *name, const char *value)
{
  PropertyEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_STRING;
#define OFF_BY_FUDGE 1

  if (value)
    {
      char *tmp = oi_malloc (strlen (value) + 1 + OFF_BY_FUDGE);
      memcpy (tmp, value, strlen (value));
      tmp[strlen(value)]=0;
      entry->o.value_string = tmp;
    }
  else
    entry->o.value_string = NULL;
  oi@"notify"((void*)name);
}
void   oi_set_oi! (var oi, const char *name, var value)
{
  PropertyEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_OI;
  if (value)
    entry->o.value_oi = value@ref:inc();
  else
    entry->o.value_oi = NULL;
  oi@"notify"((void*)name);
}

void   oi_set_pointer! (var oi, const char *name, void       *value)
{
  PropertyEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_POINTER;
  entry->o.value_pointer = value;
  oi@"notify"((void*)name);
}

float  oi_get_float! (var oi, const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->o.value_int;
      case OI_PTYPE_FLOAT:  return entry->o.value_float;
      case OI_PTYPE_STRING: return atof(entry->o.value_string);
      default: return 0.0;
    }
}

int    oi_get_int! (var oi, const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->o.value_int;
      case OI_PTYPE_FLOAT:  return entry->o.value_float;
      case OI_PTYPE_STRING: return atof(entry->o.value_string);
      default: return 0.0;
    }
}

const char *oi_get_string! (var oi, const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return "(int)";   /* XXX: printf it?, but const? */
      case OI_PTYPE_FLOAT:  return "(float)"; /* XXX: .....  */
      case OI_PTYPE_STRING: return entry->o.value_string;
      case OI_PTYPE_POINTER:return "(pointer)";
      default:              return "()";
    }
}

var oi_get_oi! (var oi, const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_OI: return (entry->o.value_oi@ref:inc());
      default:          return NULL;
    }
}

void  *oi_get_pointer!     (var oi, const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_POINTER: return entry->o.value_pointer;
      default: return NULL;
    }
}

int is_string (const char *name)
{
  PropertyEntry *entry = oi_get_entry_read (self, name);
  if (!entry)
    return 0;
  return entry->type == OI_PTYPE_STRING;
}

@end

