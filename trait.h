/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef _TRAIT_H_
#define  _TRAIT_H_
extern Type *TRAIT;
int trait_check (Var *self, Type *trait);
void *trait_get (Var *self, Type *trait);
void *trait_get_assert (Var *self, Type *trait);
void *trait_ensure (Var *self, Type *trait, Var *args);
void trait_add (Var *self, Type *type, Var *args);
void trait_remove (Var *self, Type *trait);
void trait_finalize (Var *self);
void **trait_list (Var *self, int *count);
#endif
