#include <stdlib.h>
#include "oi.h"

@generateheader

@trait Value
{
  PropertyType  type;
  union {
    float       as_float;
    double      as_double;
    int         as_int;
    const char *as_string;
    void       *as_pointer;
    var         as_oi;      /* in code.. using the oi directly
                             * if possible - likely will be more
                             * elegant..
                             */
  } value;
} PACKED;

static void scrub ()
{
  switch (this->type)
  {
    case OI_PTYPE_OI:
      /**/
      this->value.as_oi@ref:dec();
      break;
    case OI_PTYPE_STRING:
      oi_strfree (this->value.as_string);
      break;
    default:
      break;
  }
  this->type = OI_PTYPE_INT;
  this->value.as_int = 0;
}

PropertyType type ()
{
  if (!self)
    return 0;
  return this->type;
}

void set_double (double value)
{
  self@value:scrub();
  this->value.as_float = value;
  this->type = OI_PTYPE_DOUBLE;
}

void set_float (float       value)
{
  self@value:scrub();
  this->value.as_float = value;
  this->type = OI_PTYPE_FLOAT;
}

void set_int (int         value)
{
  self@value:scrub();
  this->value.as_int = value;
  this->type = OI_PTYPE_INT;
}

void set_string (const char *value)
{
  self@value:scrub();
  this->type = OI_PTYPE_STRING;
  if (value)
    this->value.as_string = oi_strdup (value);
  else
    this->value.as_string = NULL;
}
void set_oi (var value)
{
  self@value:scrub();

  if (value@trait:get (VALUE)) /* if we're setting something that already is
                                * a value.. 
                                */
    {
      switch (value@value:type())
        {
          case OI_PTYPE_INT:/**/
            self@value:set_int (value@value:get_int());
            break;
          case OI_PTYPE_FLOAT:/**/
            self@value:set_float (value@value:get_float());
            break;
          case OI_PTYPE_DOUBLE:/**/
            self@value:set_double (value@value:get_double());
            break;
          case OI_PTYPE_STRING:/**/
            self@value:set_string (value@value:get_string());
            break;
          case OI_PTYPE_POINTER:/**/
            self@value:set_pointer (value@value:get_pointer ());
            break;
          case OI_PTYPE_OI:/**/
            self@value:set_oi (value@value:get_oi ());
            break;
        }
      return;
    }

  this->type = OI_PTYPE_OI;
  if (value)
    this->value.as_oi = value@ref:inc();
  else
    this->value.as_oi = NULL;
}

void set_pointer (void       *value)
{
  self@value:scrub();
  this->type = OI_PTYPE_POINTER;
  if (value)
    this->value.as_pointer = value;
  else
    this->value.as_pointer = NULL;
}

double get_double ()
{
  if (!self)
    return 0.0;
  switch (this->type)
    {
      case OI_PTYPE_INT:     return this->value.as_int;
      case OI_PTYPE_FLOAT:   return this->value.as_float;
      case OI_PTYPE_DOUBLE:  return this->value.as_double;
      case OI_PTYPE_STRING:  return atof(this->value.as_string);
      default: return 0.0;
    }
}

float get_float ()
{
  if (!self)
    return 0.0;
  switch (this->type)
    {
      case OI_PTYPE_INT:    return this->value.as_int;
      case OI_PTYPE_FLOAT:  return this->value.as_float;
      case OI_PTYPE_DOUBLE: return this->value.as_double;
      case OI_PTYPE_STRING: return atof(this->value.as_string);
      default: return 0.0;
    }
}

int get_int ()
{
  if (!self)
    return 0;
  switch (this->type)
    {
      case OI_PTYPE_INT:     return this->value.as_int;
      case OI_PTYPE_FLOAT:   return this->value.as_float;
      case OI_PTYPE_DOUBLE:  return this->value.as_double;
      case OI_PTYPE_STRING:  return atof(this->value.as_string);
      default: return 0.0;
    }
}

const char *get_string ()
{
  /* can possibly get away without having a lock on it.. */
  static char ret_buf[128][32];
  static int sretbuf_no = 0;
  int retbuf_no = sretbuf_no++;
  if (sretbuf_no>=128)
    sretbuf_no=0;

  if (!self)
    return "";
  switch (this->type)
    {
      case OI_PTYPE_STRING: return this->value.as_string;
      case OI_PTYPE_INT:
        sprintf(ret_buf[retbuf_no], "%i", this->value.as_int);
        return ret_buf[retbuf_no];
      case OI_PTYPE_FLOAT:
        sprintf(ret_buf[retbuf_no], "%f", this->value.as_float);
        return ret_buf[retbuf_no];
      case OI_PTYPE_DOUBLE:
        sprintf(ret_buf[retbuf_no], "%f", this->value.as_double);
        return ret_buf[retbuf_no];
      case OI_PTYPE_POINTER:
        sprintf(ret_buf[retbuf_no], "<%p>", this->value.as_pointer);
      case OI_PTYPE_OI:

        if (this->value.as_oi@trait:get(STRING))
          return (this->value.as_oi@string:get());

        sprintf(ret_buf[retbuf_no], "<oi%p>", this->value.as_oi);
        return ret_buf[retbuf_no];
      default:              return "()";
    }
}

void  *get_pointer ()
{
  if (!self)
    return NULL;
  switch (this->type)
    {
      case OI_PTYPE_POINTER: return this->value.as_pointer;
      default: return NULL;
    }
}

var get_oi ()
{
  if (!self)
    return NULL;
  switch (this->type)
    {
      case OI_PTYPE_OI: return (this->value.as_oi@ref:inc());
        break;
      default:
        return (self@ref:inc());
    }
}

static void init ()
{
  this->type = OI_PTYPE_INT;
  this->value.as_int = 0;
}

static void destroy ()
{
  self@value:scrub ();
}

@end
