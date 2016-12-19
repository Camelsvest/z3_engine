#include <stdio.h>
#include "z3_hash.h"

int main(int argc, char *argv[])
{
        int index;
        z3_hash_t *table;
        z3_hash_key_t key;
        unsigned int max_length;

        table = z3_hash_init();
        max_length = 0;

        for (index = 0; index < 50000; index++)
        {
                key = z3_hash_insert(table, (void *)index);
                if (table->buckets[key])
                {
                        if (max_length < table->buckets[key]->list_num)
                        {
                                max_length = table->buckets[key]->list_num;
                        }
                }

                printf("add key = %u, index = %d, bucket length: %u, max_length = %u\r\n",
                        key, index, table->buckets[key] ? table->buckets[key]->list_num : 0, max_length);
        }

        printf("Hash finished now.\n");

        for (index = 0; index < 50000; index++)
        {
                printf("remove key = %u, index = %d, bucket length: %u\r\n",
                        key, index, table->buckets[key] ? table->buckets[key]->list_num : 0);
                key = z3_hash_remove(table, (void *)index);
        }

        z3_hash_uninit(table);
        
        return 0;
}
