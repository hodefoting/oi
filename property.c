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
} PACKED;

typedef struct
{
  const char   *name; /* quark it */
  var           value;
} PropertyEntry;

static void prop_destroy! (PropertyEntry *entry, void *user_data);
static void init ()
{
  this->props = @list:new ();
  this->props@list:set_destroy ((void*) prop_destroy, NULL);
}

static void destroy ()
{
  this->props@var:finalize();
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
  if (this)
    this->props@list:each(each_wrapper, args);
}


static void
add_to_list! (void *item, void *value, void *user_data)
{
  const char *entry = item;
  var list = user_data;
  list@list:append((void*)entry);
}

var list ()
{
  var ret = var_new (LIST, NULL);
  self@property:each(add_to_list, ret);
  //ret@list:set_destroy(oi_free, NULL);
  return ret;
}

static void prop_destroy! (PropertyEntry *entry, void *user_data)
{
  if (entry->name)
    oi_strfree ((void*)entry->name);
  ref_dec (entry->value);
  oi_free (sizeof (PropertyEntry), entry);
}

static int match_name! (void *property_entry, const char *name)
{
  PropertyEntry *entry = property_entry;
  return (entry && entry->name && name && !strcmp (entry->name, name));
}
static PropertyEntry *get_entry_read (const char *name)
{
  int no;
  PropertyEntry *entry = NULL;
  no = this->props@list:find_custom ((void*)match_name, (void*)name);
  if (no >= 0)
    entry = this->props@list:get (no);
  else
    {
#if 0
      /* reading the property creates it! */
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup ();
      this->props@list:append (entry);
      entry->type = OI_PTYPE_INT;
      entry->value.as_int = 0;
#endif
    } 
  return entry;
}

static PropertyEntry *get_entry_write (const char *name)
{
  int no;
  PropertyEntry *entry;
  self@mutex:lock ();
  no = this->props@list:find_custom((void*)match_name, (void*)name);
  if (no >= 0)
    {
      entry = this->props@list:get(no);
    }
  else
    {
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup();
      entry->value = var_new(VALUE, NULL);
      this->props@list:append (entry);
    } /* XXX: oicc hack*/ ;
  self@mutex:unlock ();
  return entry;
}

void set_float (const char *name, float       value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->value@value:type() != OI_PTYPE_FLOAT ||
      (entry->value@value:type() == OI_PTYPE_FLOAT &&
       entry->value@value:get_float() != value))

  {
    entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->value@value:set_float (value);
    self@"notify"((void*)name);
  }
}

void set_double (const char *name, double  value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->value@value:type() != OI_PTYPE_DOUBLE ||
      (entry->value@value:type() == OI_PTYPE_DOUBLE &&
       entry->value@value:get_double () != value))

  {
    entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->value@value:set_double (value);
    self@"notify"((void*)name);
  }
}

void set_int (const char *name, int         value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  PropertyType type = 0;

  if (entry)
    type = entry->value@value:type();

  if (!entry ||
      type != OI_PTYPE_INT ||
      (type == OI_PTYPE_INT &&
       entry->value@value:get_int() != value))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->value@value:set_int (value);
      self@"notify"((void*)name);
    }
}
void set_string (const char *name, const char *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  PropertyType type = 0;

  if (entry)
    type = entry->value@value:type();

  if (!entry ||
      type != OI_PTYPE_STRING ||
      (type == OI_PTYPE_STRING && strcmp(entry->value@value:get_string(), value)))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->value@value:set_string (value);
      self@"notify"((void*)name);
    }
}
void set_oi (const char *name, var value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  PropertyType type = 0;

  if (entry)
    type = entry->value@value:type();

  if (!entry ||
      type != OI_PTYPE_OI ||
      (type == OI_PTYPE_OI && entry->value@value:get_oi()!= value))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->value@value:set_oi (value);
      self@"notify"((void*)name);
    }
}

void set_pointer (const char *name, void       *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  PropertyType type = 0;

  if (entry)
    type = entry->value@value:type();

  if (!entry ||
      type != OI_PTYPE_POINTER ||
      (type == OI_PTYPE_POINTER && entry->value@value:get_pointer()!= value))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->value@value:set_pointer (value);
      self@"notify"((void*)name);
    }
}

float get_float (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0.0;
  return (entry->value@value:get_float());
}

float get_double (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0.0;
  return (entry->value@value:get_double ());
}

int get_int (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0;
  return (entry->value@value:get_int());
}

const char *get_string (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return "";
  return (entry->value@value:get_string());
}

var get_oi (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return NULL;
  return (entry->value@value:get_oi());
}

void  *get_pointer (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return NULL;
  return (entry->value@value:get_pointer());
}

PropertyType type (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0;
  return (entry->value@value:type());
}

int is_string (const char *name)
{
  return (self@property:type(name) == OI_PTYPE_STRING);
}

void dup (var clone)
{
  var proplist = self@property:list();
  int i ;
  for (i = 0; i < proplist@list:get_size(); i++)
    {
      const char *name = proplist@list:get(i);
      PropertyType type = self@property:type(name);
      switch (type)
      {
        case OI_PTYPE_FLOAT:
          property_set_float (clone, name,
          property_get_float (self, name));
          break;
        case OI_PTYPE_DOUBLE:
          property_set_double (clone, name,
          property_get_double (self, name));
          break;
        case OI_PTYPE_INT:
          property_set_int (clone, name,
          property_get_int (self, name));
          break;
        case OI_PTYPE_STRING:
          property_set_string (clone, name,
          property_get_string (self, name));
          break;
        case OI_PTYPE_OI:
          {
            var oi = property_get_oi (self, name);
            property_set_oi (clone, name, oi);
            oi@ref:dec ();
          }
          break;
        case OI_PTYPE_POINTER:
          property_set_pointer (clone, name,
          property_get_pointer (self, name));
          break;
        default:
          fprintf (stderr, "cloning of property '%s' failed due to type %i %i\n",
              name, type, OI_PTYPE_OI);
      }
    }
  proplist@ref:dec ();
}

@end

