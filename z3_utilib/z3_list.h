#ifndef _Z3_LIST_H_
#define _Z3_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Z3_LIST z3_list_t;
struct _Z3_LIST {
        void            *data;
        z3_list_t       *next, *prev;

        unsigned int    list_num;
};

typedef int (*Z3CompareCustom)(void *data_orig, void *data_custom);
typedef void (*Z3VisitCustom)(void *data_orig, void *data_custom);

z3_list_t *z3_list_add_head(z3_list_t *list, void *data);
z3_list_t *z3_list_add_tail(z3_list_t *list, void *data);
z3_list_t *z3_list_remove(z3_list_t *list, void *data);
z3_list_t *z3_list_remove_all(z3_list_t *list, void *data);
z3_list_t *z3_list_remove_link(z3_list_t *list, z3_list_t *link);
z3_list_t *z3_list_delete_link(z3_list_t *list, z3_list_t *link);
z3_list_t *z3_list_concat(z3_list_t *list1, z3_list_t *list2);
z3_list_t *z3_list_find(z3_list_t *list, void *data);
z3_list_t *z3_list_find_custom(z3_list_t *list, void *data, Z3CompareCustom func);

void z3_list_foreach(z3_list_t *list, Z3VisitCustom func, void *data);
void z3_list_free_1(z3_list_t *list);    /* �ͷŵ������ */
void z3_list_free(z3_list_t *list);      /* �ͷ����н�� */

z3_list_t *z3_list_first(z3_list_t *list);
z3_list_t *z3_list_last(z3_list_t *list);
unsigned int z3_list_number(z3_list_t *list);

#define z3_list_data(list) (((z3_list_t*)list)->data)
#define z3_list_next(list) (list ? ((z3_list_t*)list)->next : NULL)
#define z3_list_prev(list) (list ? ((z3_list_t*)list)->prev : NULL)

#ifdef __cplusplus
}
#endif

#endif