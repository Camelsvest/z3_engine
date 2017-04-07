#ifndef _Z3_MEMORY_OBJ_HPP_
#define _Z3_MEMORY_OBJ_HPP_

namespace Z3 {

        class MemoryObject
        {
        public:
                MemoryObject();
                virtual ~MemoryObject();

                static void*   operator new(size_t nSize, char *pszFilename, unsigned int nLine);
                static void    operator delete(void *p, char *pszFilename, unsigned int nLine);

                static void*   operator new[](size_t nSize, char *pszFilename, unsigned int nLine);
                static void    operator delete[](void *p, char *pszFilename, unsigned int nLine);

                static void*   operator new(size_t nSize);
                static void    operator delete(void *p);

                static void*   operator new[](size_t nSize);
                static void    operator delete[](void *p);
        };

};

#ifdef Z3_MEM_DEBUG
#define z3_debug_new new(__FILE__, __LINE__)

void*   operator new(size_t size, char *filename, unsigned int line);
void    operator delete(void* p, char *filename, unsigned int line);

void*   operator new[](size_t size, char *filename, unsigned int line);
void    operator delete[](void* p, char *filename, unsigned int line);
#endif

#endif
