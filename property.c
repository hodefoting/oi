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

typedef struct
{
  PropertyType  type;
  const char   *name; /* quark it */
  union {
    float       as_float;
    int         as_int;
    const char *as_string;
    void       *as_pointer;
    var         as_oi;
  } value;
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

static void prop_unset! (PropertyEntry *entry)
{
  if (entry->type == OI_PTYPE_STRING && entry->value.as_string)
    {
      oi_strfree ((void*)entry->value.as_string);
      entry->value.as_string = NULL;
    }
  else if (entry->type == OI_PTYPE_OI && entry->value.as_oi)
    {
      entry->value.as_oi@ref:dec ();
      entry->value.as_oi = NULL;
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
      prop_unset (entry);
    }
  else
    {
      entry = oi_malloc (sizeof (PropertyEntry));
      entry->name = name@oi:strdup();
      entry->value.as_int = 0;
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
      (entry->type == OI_PTYPE_FLOAT && entry->value.as_float != value))

  {
    entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->value.as_float = value;
    entry->type = OI_PTYPE_FLOAT;
    self@"notify"((void*)name);
  }
}

void set_int (const char *name, int         value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_INT ||
      (entry->type == OI_PTYPE_INT && entry->value.as_int != value))
    {
      self@"pre-notify"((void*)name);
      entry = self@property:get_entry_write (name);
      entry->type = OI_PTYPE_INT;
      entry->value.as_int = value;
      self@"notify"((void*)name);
    }
}
void set_string (const char *name, const char *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_STRING ||
      (entry->type == OI_PTYPE_STRING && strcmp(entry->value.as_string, value)))
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
          entry->value.as_string = tmp;
        }
      else
        entry->value.as_string = NULL;
      self@"notify"((void*)name);
      }
}
void set_oi (const char *name, var value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_OI ||
      (entry->type == OI_PTYPE_OI && entry->value.as_oi != value))
  {
    self@"pre-notify"((void*)name);
    entry = self@property:get_entry_write (name);
    entry->type = OI_PTYPE_OI;
    if (value)
      entry->value.as_oi = value@ref:inc();
    else
      entry->value.as_oi = NULL;
    self@"notify"((void*)name);
  }
}

void set_pointer (const char *name, void       *value)
{
  PropertyEntry *entry = self@property:get_entry_read (name);

  if (!entry ||
      entry->type != OI_PTYPE_POINTER || 
      (entry->type == OI_PTYPE_POINTER && entry->value.as_pointer != value))
  {
    PropertyEntry *entry = self@property:get_entry_write (name);
    self@"pre-notify"((void*)name);
    entry->type = OI_PTYPE_POINTER;
    entry->value.as_pointer = value;
    self@"notify"((void*)name);
  }
}

float get_float (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0.0;
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->value.as_int;
      case OI_PTYPE_FLOAT:  return entry->value.as_float;
      case OI_PTYPE_STRING: return atof(entry->value.as_string);
      default: return 0.0;
    }
}

int get_int (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0;
  switch (entry->type)
    {
      case OI_PTYPE_INT:    return entry->value.as_int;
      case OI_PTYPE_FLOAT:  return entry->value.as_float;
      case OI_PTYPE_STRING: return atof(entry->value.as_string);
      default: return 0.0;
    }
}

const char *get_string (const char *name)
{
  /* can possibly get away without having a lock on it.. */
  static char ret_buf[128][32];
  static int sretbuf_no = 0;
  int retbuf_no = sretbuf_no++;
  if (sretbuf_no>=128)
    sretbuf_no=0;

  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return "";
  switch (entry->type)
    {
      case OI_PTYPE_STRING: return entry->value.as_string;
      case OI_PTYPE_INT:
        sprintf(ret_buf[retbuf_no], "%i", entry->value.as_int);
        return ret_buf[retbuf_no];
      case OI_PTYPE_FLOAT:
        sprintf(ret_buf[retbuf_no], "%f", entry->value.as_float);
        return ret_buf[retbuf_no];
      case OI_PTYPE_POINTER:
        sprintf(ret_buf[retbuf_no], "<%p>", entry->value.as_pointer);
      case OI_PTYPE_OI:
        sprintf(ret_buf[retbuf_no], "<oi%p>", entry->value.as_oi);
        return ret_buf[retbuf_no];
      default:              return "()";
    }
}

var get_oi (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return NULL;
  switch (entry->type)
    {
      case OI_PTYPE_OI: return (entry->value.as_oi@ref:inc());
      default:          return NULL;
    }
}

void  *get_pointer (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return NULL;
  switch (entry->type)
    {
      case OI_PTYPE_POINTER: return entry->value.as_pointer;
      default: return NULL;
    }
}

PropertyType type (const char *name)
{
  PropertyEntry *entry = self@property:get_entry_read (name);
  if (!entry)
    return 0;
  return entry->type;
}

int is_string (const char *name)
{
  return (self@property:type(name) == OI_PTYPE_STRING);
}

@end

