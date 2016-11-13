#ifndef _Z3_ALLOC_H_
#define _Z3_ALLOC_H_

#if defined(Z3_MEM_DEBUG) || defined(_DEBUG) || defined(DEBUG)
        #define z3_alloc_init()         _z3_alloc_init()
        #define z3_alloc_uninit()       _z3_alloc_uninit()

        #define z3_malloc(size)         _z3_malloc(size, __FILE__, __LINE__)
        #define z3_calloc(a, b)         _z3_calloc(a, b, __FILE__, __LINE__)
        #define z3_free(p)              _z3_free(p)

        #ifdef __cplusplus
        extern "C" {
        #endif

        void    _z3_alloc_init();
        void    _z3_alloc_uninit();

        void*   _z3_malloc(size_t size, char *filename, unsigned int line);
        void*   _z3_calloc(size_t num, size_t size, char *filename, unsigned int line);
        void    _z3_free(void *memblock);


        #ifdef __cplusplus
        }
        #endif
#else
        #define z3_alloc_init()
        #define z3_alloc_uninit()
        #define z3_malloc(size)         malloc(size)
        #define z3_calloc(a, b)         calloc(a, b)
        #define z3_free(p)              free(p)

        #define _z3_malloc(a, b, c)     malloc(a)
        #define _z3_calloc(a, b, c, d)  calloc(a, b)
        #define _z3_free(p)             free(p)
#endif

#endif