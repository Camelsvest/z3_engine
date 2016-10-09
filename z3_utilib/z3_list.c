#include <Windows.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include "z3_list.h"

static z3_list_t* z3_list_alloc(void)
{
        z3_list_t* list = (z3_list_t *)calloc(1, sizeof(z3_list_t));

        return list;
}

z3_list_t *z3_list_add(z3_list_t *list, void *data)
{
        z3_list_t *new_list;

        new_list = z3_list_alloc();
        assert(new_list);

        new_list->data = data;
        new_list->prev = list;
        new_list->next = NULL;

        if (list)
                list->next = new_list;

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
		last->next = new_list;
		return list;
	}
	else
	{
		new_list->prev = NULL;
		return new_list;
	}
}

z3_list_t *z3_list_remove(z3_list_t *list, void *data)
{
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
				head = head->next;

			z3_list_free_1(list);
			break;
		}
	}

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
				head = next;

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
			list = list->next;

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
			last->next = list2;
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
	Z2CompareCustom func)
{
	while (list)
	{
		if (!(*func)(list->data, data))
			break;
		list = list->next;
	}

	return list;
}


void z3_list_foreach(z3_list_t *list, Z2VisitCustom func, void *data)
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
