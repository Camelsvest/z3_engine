#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "z3_alloc.h"
#include "z3_trace.h"
#include "z3_list.h"

#define MAX_BUF_SIZE	256
#define MAX_THREADS	5

void* thread_func(void *args)
{
	FILE	*fp;
	char	*buf, *dupbuf;
	ssize_t	read_bytes;
	size_t	buf_size;
	int     index = 0;

	fp = fopen("./sample.txt", "r");
	if (!fp)
	{
		printf("Failed to open file ./sample.txt\r\n");
		goto end2;
	}

	buf_size = MAX_BUF_SIZE;
	buf = (char *)z3_malloc(buf_size);
	if (!buf)
	{
		TRACE_ERROR("Failed to allocate %d bytes memory\r\n", MAX_BUF_SIZE);
		goto end1;
	}

	read_bytes = getline(&buf, &buf_size, fp);
	while (read_bytes > 0)
	{
		assert(read_bytes < buf_size);
		buf[read_bytes] = '\0';

		dupbuf = z3_strdup(buf);
		TRACE_INFO("%s", dupbuf);
		z3_free(dupbuf);

		++index;
		buf = (char *)z3_realloc(buf, buf_size + index);
		read_bytes = getline(&buf, &buf_size, fp);
	}

end1:
	fclose(fp);
	z3_free(buf);

end2:
	TRACE_NOTICE("Thread %d quit\r\n", (int)pthread_self());
	return 0;
}

int test_list()
{
        z3_list_t *list;
        int index;

        list = NULL;
        for (index = 0; index < 10000; index++)
        {
                if (index % 2)
                {
                        list = z3_list_add_head(list, (void *)index);
                }
                else
                {
                        list = z3_list_add_tail(list, (void *)index);
                }

                printf("Add index %d, list num:%u\r\n", index, list->list_num);
        }

        printf("Add finished \r\n");

        for (index = 10000; index >= 0; --index)
        {
                printf("Remove index %d, list num:%u\r\n", index, list->list_num);

                list = z3_list_remove_all(list, (void *)index);
        }

        assert(list == NULL);

        return 0;
}

int test_trace()
{
	pthread_t	thread_ids[MAX_THREADS];
	int		error, index = 0;
	void		*retval = NULL;

	TRACE_INIT(LOG_DETAIL);
	z3_alloc_init();

	while (index < MAX_THREADS)
	{
		error = pthread_create(&thread_ids[index], NULL, thread_func, NULL);
		assert(error == 0);

		++index;
	}

	//getchar();

	index = 0;
	while (index < MAX_THREADS)
	{
		error = pthread_join(thread_ids[index], &retval);
		TRACE_NOTICE("pthread_join return %d, index = %d\r\n", error, index);

		++index;
	}

	z3_alloc_uninit();
	TRACE_UNINIT();

        return 0;
}

int main(int argc, char *argv[])
{

        //test_list();
        test_trace();
	return 0;
}
