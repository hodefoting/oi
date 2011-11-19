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

/* should probably a b-tree instead of a list to improve performance */

#include "oi.h"
#include "stdlib.h"

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
    Oi         *value_oi;
  };
} PropertiesEntry;

typedef struct
{
  OiCapability capability;
  Oi    *props;
} Properties;

static void prop_destroy (PropertiesEntry *entry, void *user_data);
static void properties_init (Oi *oi, OiCapability *capability, Oi *args)
{
  Properties *properties = (Properties*)capability;
  properties->props = oi_list_new ();
  oi_list_set_destroy (properties->props, (void*) prop_destroy, NULL);
}

static void properties_destroy (Oi *oi, OiCapability *capability)
{
  Properties *properties = (Properties*)capability;
  oi_destroy (properties->props);
}
OI(PROPERTIES, Properties, NULL, properties_init, properties_destroy)

static void prop_unset (PropertiesEntry *entry)
{
  if (entry->type == OI_PTYPE_STRING && entry->value_string)
    {
      oi_strfree ((void*)entry->value_string);
      entry->value_string = NULL;
    }
  else if (entry->type == OI_PTYPE_OI && entry->value_oi)
    {
      oi_unref (entry->value_oi);
      entry->value_oi = NULL;
    }
}

static void prop_destroy (PropertiesEntry *entry, void *user_data)
{
  prop_unset (entry);
  if (entry->name)
    oi_strfree ((void*)entry->name);
  oi_free (sizeof (PropertiesEntry), entry);
}

static int match_name (void *properties_entry, const char *name)
{
  PropertiesEntry *entry = properties_entry;
  return (entry && entry->name && name && !strcmp (entry->name, name));
}

static PropertiesEntry *oi_get_entry_read (Oi *oi, const char *name)
{
  Properties *properties = ((void*)oi_capability_ensure (oi, PROPERTIES, NULL));
  int no;
  PropertiesEntry *entry;
  no = oi_list_find_custom (properties->props, (void*)match_name, (void*)name);
  if (no >= 0)
    entry = oi_list_get (properties->props, no);
  else
    {
      entry = oi_malloc (sizeof (PropertiesEntry));
      entry->name = oi_strdup (name);
      oi_list_append (properties->props, entry);
      entry->type = OI_PTYPE_INT;
      entry->value_int = 0;
    }
  return entry;
}

static PropertiesEntry *oi_get_entry_write (Oi *oi, const char *name)
{
  Properties *properties = ((void*)oi_capability_ensure (oi, PROPERTIES, NULL));
  int no;
  PropertiesEntry *entry;
  no = oi_list_find_custom (properties->props, (void*)match_name, (void*)name);
  if (no >= 0)
    {
      entry = oi_list_get (properties->props, no);
      prop_unset (entry);
    }
  else
    {
      entry = oi_malloc (sizeof (PropertiesEntry));
      entry->name = oi_strdup (name);
      entry->value_int = 0;
      oi_list_append (properties->props, entry);
      entry->type = OI_PTYPE_INT;
    }
  return entry;
}


void   oi_set_float       (Oi *oi, const char *name, float       value)
{
  PropertiesEntry *entry = oi_get_entry_write (oi, name);
  int changed = entry->value_float != value;
  entry->value_float = value;
  entry->type = OI_PTYPE_FLOAT;
  if (changed)
    oi_message_emit (oi, "notify", (void*)name);
}

void   oi_set_int         (Oi *oi, const char *name, int         value)
{
  PropertiesEntry *entry = oi_get_entry_write (oi, name);
  int changed = entry->value_int != value;
  entry->type = OI_PTYPE_INT;
  entry->value_int = value;
  if (changed)
    oi_message_emit (oi, "notify", (void*)name);
}
void   oi_set_string      (Oi *oi, const char *name, const char *value)
{
  PropertiesEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_STRING;
  if (value)
    entry->value_string = oi_strdup (value);
  else
    entry->value_string = NULL;
  oi_message_emit (oi, "notify", (void*)name);
}
void   oi_set_oi          (Oi *oi, const char *name, Oi *value)
{
  PropertiesEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_OI;
  if (value)
    entry->value_oi = oi_ref (value);
  else
    entry->value_oi = NULL;
  oi_message_emit (oi, "notify", (void*)name);
}

void   oi_set_pointer     (Oi *oi, const char *name, void       *value)
{
  PropertiesEntry *entry = oi_get_entry_write (oi, name);
  entry->type = OI_PTYPE_POINTER;
  entry->value_pointer = value;
  oi_message_emit (oi, "notify", (void*)name);
}

float  oi_get_float       (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->value_int;
      case OI_PTYPE_FLOAT:  return entry->value_float;
      case OI_PTYPE_STRING: return atof(entry->value_string);
      default: return 0.0;
    }
}

int    oi_get_int         (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->value_int;
      case OI_PTYPE_FLOAT:  return entry->value_float;
      case OI_PTYPE_STRING: return atof(entry->value_string);
      default: return 0.0;
    }
}

const char *oi_get_string (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return "(int)";
      case OI_PTYPE_FLOAT:  return "(float)";
      case OI_PTYPE_STRING: return entry->value_string;
      case OI_PTYPE_POINTER:return "(pointer)";
      default:              return "()";
    }
}

Oi *oi_get_oi (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_OI: return oi_ref (entry->value_oi);
      default:          return NULL;
    }
}

void  *oi_get_pointer     (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  switch (entry->type)
    {
      case OI_PTYPE_POINTER: return entry->value_pointer;
      default: return NULL;
    }
}

static void
each_wrapper (void *item, void *user_data)
{
  void **args = user_data;
  PropertiesEntry *entry = item;
  void (*cb)(const char *key, void *item, void *user_data) = args[0];
  cb (entry->name, "foo", args[1]);
}

void
oi_properties_each (Oi *oi,
                    void (*cb)(const char *key, void *item, void *user_data),
                    void *user_data)
{
  void *args[] = {cb, user_data};
  Properties *properties = ((void*)oi_capability_get (oi, PROPERTIES));
  oi_list_each (properties->props, each_wrapper, args);
}

int
oi_property_is_string (Oi *oi, const char *name)
{
  PropertiesEntry *entry = oi_get_entry_read (oi, name);
  if (!entry)
    return 0;
  return entry->type == OI_PTYPE_STRING;
}
