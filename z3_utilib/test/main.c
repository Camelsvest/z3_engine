#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "z3_alloc.h"
#include "z3_trace.h"

#define MAX_BUF_SIZE	256
#define MAX_THREADS	10

void* thread_func(void *args)
{
	FILE	*fp;
	char	*buf;
	ssize_t	read_bytes;
	size_t	buf_size;

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

		TRACE_INFO("%s", buf);
		read_bytes = getline(&buf, &buf_size, fp);
	}

end1:
	fclose(fp);
	z3_free(buf);

end2:
	TRACE_NOTICE("Thread %d quit\r\n", (int)pthread_self());
	return 0;
}

int main(int argc, char *argv[])
{
	pthread_t	thread_ids[10];
	int		error, index = 0;
	void		*retval;

	TRACE_INIT(LOG_DEBUG);
	z3_alloc_init();

	while (index < MAX_THREADS)
	{
		error = pthread_create(&thread_ids[index], NULL, thread_func, NULL);
		assert(error == 0);

		++index;
	}

	getchar();

	index = 0;
	while (index < MAX_THREADS)
	{
		error = pthread_join(thread_ids[index], &retval);
		TRACE_NOTICE("pthread_join return %d\r\n", error);

		++index;
	}

	z3_alloc_uninit();
	TRACE_UNINIT();

	return 0;
}
