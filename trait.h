#ifndef O_TRAIT_H
#define  O_TRAIT_H
extern OiType *TRAIT;
int trait_check (Oi *self, OiType *trait);
void *trait_get (Oi *self, OiType *trait);
void *trait_get_assert (Oi *self, OiType *trait);
void *trait_ensure (Oi *self, OiType *trait, Oi *args);
void trait_add (Oi *self, OiType *type, Oi *args);
void trait_remove (Oi *self, OiType *trait);
void trait_finalize (Oi *self);
const OiTrait **trait_list (Oi *self, int *count);
#endif
