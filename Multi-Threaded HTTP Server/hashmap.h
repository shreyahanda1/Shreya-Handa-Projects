#ifndef HASHMAP_H
#define HASHMAP_H

#include "rwlock.h"

rwlock_t *get_rwlock(const char *uri);
void delete_hash(const char *uri);
void free_hash(void);

#endif