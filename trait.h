/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef O_TRAIT_H
#define  O_TRAIT_H
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
