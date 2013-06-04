/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef O_OISTRING_H
#define  O_OISTRING_H
extern Type *STRING;
void string_clear (Var *self);
void string_appendc (Var *self, int val);
void string_append_str (Var *self, const char *str);
void string_append_string (Var *self, Var *oi2);
const char *string_get (Var *self);
char *string_dissolve (Var *self);
void string_append_printf (Var *self, const char *format, ...);
Var *string_new (const char *initial);
#endif
