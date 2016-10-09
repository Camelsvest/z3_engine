#ifndef _Z3_ASYNC_OBJ_HPP_
#define _Z3_ASYNC_OBJ_HPP_

#define Z3_OBJ_ADDREF(p)        if (p) { p->AddRef(); }
#define Z3_OBJ_RELEASE(p)       if (p) { p->Release(); p = NULL; } 

namespace Z3 {

        class Lock
        {
        public:
                Lock();
                virtual ~Lock();

                bool    On();
                void    Off();

        private:
                HANDLE  m_hMutex;
        };

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

        class AsyncObj : public MemoryObject
        {
        public:
                AsyncObj(uint32_t nObjID = INVALID_OBJ_ID);

                inline bool     Lock()  { return m_Lock.On(); }
                inline void     Unlock()        { return m_Lock.Off(); }

                inline uint32_t GetObjID()      { return m_nObjID; }
                uint32_t        GetRefCount();

                uint32_t        AddRef();
                void            Release();

        protected:
                virtual ~AsyncObj();

        private:
                Z3::Lock        m_Lock;
                uint32_t        m_nObjID;
                int32_t         m_nRefCount;
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