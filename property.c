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
    var         value_oi;
  } o;
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
static PropertyEntry *get_entry_read (const char *name)
{
  int no;
  PropertyEntry *entry;
  no = this->props@list:find_custom ((void*)match_name, (void*)name);
  if (no >= 0)
    entry = this->props@list:get (no);
  else
    {
      /* reading the property creates it! */
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup ();
      this->props@list:append (entry);
      entry->type = OI_PTYPE_INT;
      entry->o.value_int = 0;
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
      prop_unset (entry);
    }
  else
    {
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup();
      entry->o.value_int = 0;
      this->props@list:append (entry);
      entry->type = OI_PTYPE_INT;
    } /* XXX: oicc hack*/ ;
  self@mutex:unlock ();
  return entry;
}

void set_float (const char *name, float       value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_FLOAT ||
      (entry->type == OI_PTYPE_FLOAT && entry->o.value_float != value))

  {
    entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->o.value_float = value;
    entry->type = OI_PTYPE_FLOAT;
    self@"notify"((void*)name);
  }
}

void set_int (const char *name, int         value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_INT ||
      (entry->type == OI_PTYPE_INT && entry->o.value_int != value))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->type = OI_PTYPE_INT;
      entry->o.value_int = value;
      self@"notify"((void*)name);
    }
}
void set_string (const char *name, const char *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_STRING ||
      (entry->type == OI_PTYPE_STRING && strcmp(entry->o.value_string, value)))
      {

        entry = self@property:get_entry_write (name);
        self@"pre-notify"((void*)name);
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
      self@"notify"((void*)name);
      }
}
void set_oi (const char *name, var value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_OI ||
      (entry->type == OI_PTYPE_OI && entry->o.value_oi != value))
  {
    self@"pre-notify"((void*)name);
    entry = self@property:get_entry_write (name);
    entry->type = OI_PTYPE_OI;
    if (value)
      entry->o.value_oi = value@ref:inc();
    else
      entry->o.value_oi = NULL;
    self@"notify"((void*)name);
  }
}

void set_pointer (const char *name, void       *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_POINTER || 
      (entry->type == OI_PTYPE_POINTER && entry->o.value_pointer != value))
  {
    PropertyEntry *entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->type = OI_PTYPE_POINTER;
    entry->o.value_pointer = value;
    self@"notify"((void*)name);
  }
}

float get_float (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->o.value_int;
      case OI_PTYPE_FLOAT:  return entry->o.value_float;
      case OI_PTYPE_STRING: return atof(entry->o.value_string);
      default: return 0.0;
    }
}

int get_int (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->o.value_int;
      case OI_PTYPE_FLOAT:  return entry->o.value_float;
      case OI_PTYPE_STRING: return atof(entry->o.value_string);
      default: return 0.0;
    }
}

const char *get_string (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return "(int)";   /* XXX: printf it?, but const? */
      case OI_PTYPE_FLOAT:  return "(float)"; /* XXX: .....  */
      case OI_PTYPE_STRING: return entry->o.value_string;
      case OI_PTYPE_POINTER:return "(pointer)";
      default:              return "()";
    }
}

var get_oi (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  switch (entry->type)
    {
      case OI_PTYPE_OI: return (entry->o.value_oi@ref:inc());
      default:          return NULL;
    }
}

void  *get_pointer (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  switch (entry->type)
    {
      case OI_PTYPE_POINTER: return entry->o.value_pointer;
      default: return NULL;
    }
}

int is_string (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0;
  return entry->type == OI_PTYPE_STRING;
}

@end

