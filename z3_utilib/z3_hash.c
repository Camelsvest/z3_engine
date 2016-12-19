#include <assert.h>
#include <stdlib.h>
#include "z3_hash.h"

static z3_hash_key_t fnv_random_key(unsigned int index, unsigned int down_shift, unsigned int mask)
{
        return (unsigned int)(((index * 1103515245) >> down_shift) & mask);
}

static z3_hash_key_t hash_pointer_key(z3_hash_t *hash_table, void *data)
{
        return fnv_random_key((unsigned int)data, hash_table->down_shift, hash_table->mask);
}

z3_hash_t* z3_hash_init()
{
        const unsigned int hash_size = 512;
        z3_hash_t *hash_table;

        if (hash_size > 0)
        {
                hash_table = (z3_hash_t *)calloc(1, sizeof(z3_hash_t));

                hash_table->down_shift = 22;
                hash_table->mask = 0x1FF;
                hash_table->buckets = (z3_hash_entry **)calloc(hash_size, sizeof(z3_hash_entry *));
                hash_table->bucket_size = hash_size;
                hash_table->available_num = 0;

                return hash_table;
        }

        return NULL;
}

void z3_hash_uninit(z3_hash_t *hash_table)
{
        if (hash_table)
        {
                free(hash_table->buckets);
                free(hash_table);
        }
}

z3_hash_key_t z3_hash_lookup(z3_hash_t *hash_table, void *data)
{
        unsigned int key = hash_pointer_key(hash_table, data);

        assert(key < hash_table->bucket_size);

        return key;
}

z3_hash_key_t z3_hash_insert(z3_hash_t *hash_table, void *data)
{
        z3_list_t *list_node;
        unsigned int key = z3_hash_lookup(hash_table, data);
      
        list_node = z3_list_add_head(hash_table->buckets[key], data);
        
        hash_table->buckets[key] = list_node;
        hash_table->available_num++;

        return key;
}

z3_hash_key_t z3_hash_remove(z3_hash_t *hash_table, void *data)
{
        z3_list_t *list_node;
        unsigned int key = z3_hash_lookup(hash_table, data);

        list_node = z3_list_remove(hash_table->buckets[key], data);

        hash_table->buckets[key] = list_node;
        hash_table->available_num--;

        return key;
}

unsigned int z3_hash_amount(z3_hash_t *hash_table)
{
        if (hash_table)
        {
                return hash_table->available_num;
        }
        else
                return 0;
}

void z3_hash_foreach(z3_hash_t *table, Z3VisitCustom func, void *data)
{
        z3_hash_entry *entries;
        unsigned int index;

        for (index = 0; index < table->bucket_size; index++)
        {
                if (table->buckets[index])
                {
                        entries = table->buckets[index];
                        z3_list_foreach(entries, func, NULL);
                }                               
        }
}