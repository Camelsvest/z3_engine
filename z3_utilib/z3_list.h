#ifndef _Z3_LIST_H_
#define _Z3_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Z3_LIST z3_list_t;
struct _Z3_LIST {
        void            *data;
        z3_list_t       *next, *prev;
};

typedef int (*Z2CompareCustom)(void *data_orig, void *data_custom);
typedef void (*Z2VisitCustom)(void *data_orig, void *data_custom);

z3_list_t *z3_list_add(z3_list_t *list, void *data);
z3_list_t *z3_list_add_tail(z3_list_t *list, void *data);
z3_list_t *z3_list_remove(z3_list_t *list, void *data);
z3_list_t *z3_list_remove_all(z3_list_t *list, void *data);
z3_list_t *z3_list_remove_link(z3_list_t *list, z3_list_t *link);
z3_list_t *z3_list_delete_link(z3_list_t *list, z3_list_t *link);
z3_list_t *z3_list_concat(z3_list_t *list1, z3_list_t *list2);
z3_list_t *z3_list_find(z3_list_t *list, void *data);
z3_list_t *z3_list_find_custom(z3_list_t *list, void *data, Z2CompareCustom func);

void z3_list_foreach(z3_list_t *list, Z2VisitCustom func, void *data);
void z3_list_free_1(z3_list_t *list);    /* 释放单个结点 */
void z3_list_free(z3_list_t *list);      /* 释放所有结点 */

z3_list_t *z3_list_first(z3_list_t *list);
z3_list_t *z3_list_last(z3_list_t *list);

#define z3_list_data(list) (((z3_list_t*)list)->data)
#define z3_list_next(list) (list ? ((z3_list_t*)list)->next : NULL)
#define z3_list_prev(list) (list ? ((z3_list_t*)list)->prev : NULL)

#ifdef __cplusplus
}
#endif

#endif