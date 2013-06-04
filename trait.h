/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef O_TRAIT_H
#define  O_TRAIT_H
extern Type *TRAIT;
int trait_check (Oi *self, Type *trait);
void *trait_get (Oi *self, Type *trait);
void *trait_get_assert (Oi *self, Type *trait);
void *trait_ensure (Oi *self, Type *trait, Oi *args);
void trait_add (Oi *self, Type *type, Oi *args);
void trait_remove (Oi *self, Type *trait);
void trait_finalize (Oi *self);
const OiTrait **trait_list (Oi *self, int *count);
#endif
