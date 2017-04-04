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

        class RefObj : public MemoryObject
        {
        public:
                RefObj();

                inline uint32_t GetRefCount();

                inline uint32_t AddRef();
                inline void     Release(bool bFree = true);

        protected:
                virtual ~RefObj();

        private:
                int32_t         m_nRefCount;
        };

        /*
         * Description: 全局Timer对象，负责所有timer的创建和删除
         */
        class TimerEngine : public RefObj
        {
        public:
                static TimerEngine*     Instance();
                static void             Destroy();

                HANDLE  AddTimer(PVOID lpParameter, uint32_t millseconds, bool bRepeat = false);
                bool    DeleteTimer(uint32_t nObjID, HANDLE hTimer);

        protected:
                TimerEngine();
                ~TimerEngine();

                static void CALLBACK TimerCallBack(PVOID lpParameter, BOOLEAN bTimerExpired);

        private:
                static TimerEngine*     m_pInstance;
                static HANDLE           m_hMutex;

                static HANDLE           m_hTimerQueue;
        };

        
        class AsyncObj : public RefObj
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
        };

        class TimerObj : public AsyncObj
        {
        public:
                TimerObj(uint32_t nObjID = INVALID_OBJ_ID);

                bool AddTimer(uint32_t nTimerID, void *pData, uint32_t millseconds, bool bRepeat = false);
                bool DeleteTimer(uint32_t nTimerID);

                virtual void    OnTimer(uint32_t nTimerID, void *pData);

        protected:
                virtual ~TimerObj();

        private:
                TimerEngine *m_pTimerEngine;
                std::map<uint32_t, HANDLE> m_mapTimerHandle;
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