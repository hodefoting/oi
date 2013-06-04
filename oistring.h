#ifndef O_OISTRING_H
#define  O_OISTRING_H
extern OiType *STRING;
void string_clear (Oi *self);
void string_appendc (Oi *self, int val);
void string_append_str (Oi *self, const char *str);
void string_append_string (Oi *self, Oi *oi2);
const char *string_get (Oi *self);
char *string_dissolve (Oi *self);
void string_append_printf (Oi *self, const char *format, ...);
#endif
