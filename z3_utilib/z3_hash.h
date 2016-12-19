#ifndef _Z3_HASH_H_
#define _Z3_HASH_H_

#include "z3_list.h"

typedef z3_list_t z3_hash_entry;
typedef unsigned int z3_hash_key_t;

typedef struct _z3_hash_t z3_hash_t;
struct _z3_hash_t {
        unsigned int        bucket_size;

        unsigned int        down_shift;
        unsigned int        mask;

        z3_hash_entry       **buckets;
        unsigned int        available_num;
};

z3_hash_t* z3_hash_init();
void z3_hash_uninit(z3_hash_t *hash_table);

z3_hash_key_t z3_hash_lookup(z3_hash_t *hash_table, void *data);
z3_hash_key_t z3_hash_insert(z3_hash_t *hash_table, void *data);
z3_hash_key_t z3_hash_remove(z3_hash_t *hash_table, void *data);
unsigned int z3_hash_amount(z3_hash_t *hash_table);

void z3_hash_foreach(z3_hash_t *table, Z3VisitCustom func, void *data);

#endif
