
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include "z3_list.h"

static z3_list_t* z3_list_alloc(void)
{
        z3_list_t* list = (z3_list_t *)calloc(1, sizeof(z3_list_t));

        return list;
}

z3_list_t *z3_list_add_head(z3_list_t *list, void *data)
{
        z3_list_t *new_list;

        new_list = z3_list_alloc();
        assert(new_list);

        new_list->data = data;
        new_list->prev = NULL;

        if (list)
        {
                new_list->next = list;
                new_list->list_num = list->list_num + 1;

                list->prev = new_list;
                list->list_num = 0;
        }
        else
        {
                new_list->next = NULL;
                new_list->list_num = 1;
        }                

        return new_list;
}

z3_list_t *z3_list_add_tail(z3_list_t *list, void *data)
{
	z3_list_t *new_list = z3_list_alloc();
	z3_list_t *last;

	new_list->data = data;
	new_list->next = NULL;

	if (list)
	{
		last = z3_list_last(list);
                
		new_list->prev = last;
                list->list_num += 1;
                
		last->next = new_list;

		return list;
	}
	else
	{
		new_list->prev = NULL;
                new_list->list_num = 1;

		return new_list;
	}
}

z3_list_t *z3_list_remove(z3_list_t *list, void *data)
{
#if defined(DEBUG) || defined(_DEBUG)
        int     removed = 0;
#endif
	z3_list_t *head = list;

	while (list)
	{
		if (list->data != data)
			list = list->next;
		else
		{
			if (list->prev)
				list->prev->next = list->next;
			if (list->next)
				list->next->prev = list->prev;

			if (head == list)
                        {
				head = head->next;
                                if (head)
                                        head->list_num = list->list_num;
                        }

			z3_list_free_1(list);

                        if (head)
                                head->list_num -= 1;

                #if defined(DEBUG) || defined(_DEBUG)
                        removed = 1;
                #endif
			break;
		}
	}

#if defined(DEBUG) || defined(_DEBUG)
        assert(removed);
#endif

	return head;
}


z3_list_t *z3_list_remove_all(z3_list_t *list, void *data)
{
	z3_list_t *next, *head = list;

	while (list)
	{
		if (list->data != data)
			list = list->next;
		else
		{
			next = list->next;

			if (list->prev)
				list->prev->next = next;
			else
                        {
                                if (next)
                                {
                                        next->list_num = head->list_num;
                                }

				head = next;
                        }

                        if (head)
                        {
                                head->list_num -= 1;
                        }

			if (next)
				next->prev = list->prev;

			z3_list_free_1(list);
			list = next;                       
		}
	}

	return head;	
}


z3_list_t *z3_list_remove_link(z3_list_t *list, z3_list_t *link)
{
	if (link)
	{
		if (link->next)
			link->next->prev = link->prev;
		if (link->prev)
			link->prev->next = link->next;

		if (link == list)
                {
                        list->next->list_num = list->list_num;
			list = list->next;
                }

                if (list)
                {
                        list->list_num -= 1;
                }

		link->next = NULL;
		link->prev = NULL;
	}

	return list;
}


z3_list_t *z3_list_delete_link(z3_list_t *list, z3_list_t *link)
{
	list = z3_list_remove_link(list, link);
	z3_list_free_1(link);

	return list;
}


z3_list_t *z3_list_concat(z3_list_t *list1, z3_list_t *list2)
{
	z3_list_t *last;

	if (list2)
	{
		last = z3_list_last(list1);
		if (last)
                {
			last->next = list2;
                        list1->list_num += list2->list_num;
                        list2->list_num = 0;
                }
		else
			list1 = list2;

		list2->prev = last;
	}

	return list1;	
}


z3_list_t *z3_list_find(z3_list_t *list, void *data)
{
	while (list)
	{
		if (list->data == data)
			break;
		list = list->next;
	}

	return list;
}


z3_list_t *z3_list_find_custom(z3_list_t *list, void *data,
	Z3CompareCustom func)
{
	while (list)
	{
		if (!(*func)(list->data, data))
			break;
		list = list->next;
	}

	return list;
}


void z3_list_foreach(z3_list_t *list, Z3VisitCustom func, void *data)
{
	z3_list_t *next;

	while (list)
	{
		next = list->next;
		(*func)(list->data, data);
		list = next;
	}
}


void z3_list_free_1(z3_list_t *list)
{
	if (list)
		free(list);
}


void z3_list_free(z3_list_t *list)
{
	z3_list_t *next;

	while (list)
	{
		next = list->next;
		z3_list_free_1(list);
		list = next;
	}
}



z3_list_t *z3_list_first(z3_list_t *list)
{
	if (list)
	{
		while (list->prev)
			list = list->prev;
	}

	return list;
}


z3_list_t *z3_list_last(z3_list_t *list)
{
	if (list)
	{
		while (list->next)
			list = list->next;
	}

	return list;
}

unsigned int z3_list_number(z3_list_t *list)
{
        if (list)
        {
                return list->list_num;
        }
        else
                return 0;
}