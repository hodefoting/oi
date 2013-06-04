#ifndef O_MUTEX_H
#define  O_MUTEX_H
Oi *mutex_lock (Oi *self);
int mutex_trylock (Oi *self);
void mutex_unlock (Oi *self);
extern OiType *MUTEX;
#endif
