#ifndef _Z3_THREAD_HPP_
#define _Z3_THREAD_HPP_

#include "z3_obj.hpp"

namespace Z3 {

        class Thread : public AsyncObj
        {
        public:
                Thread(uint32_t nObjID = INVALID_OBJ_ID);

                bool    Start(void);
                void    Stop(void);

        protected:
                virtual ~Thread();

                static unsigned __stdcall ThreadFunc(void *pArgs);
                
                virtual bool    OnThreadStart(void);
                virtual void    RunOnce() = 0;
                virtual void    OnThreadStop(void);

        private:
                HANDLE          m_hThread;
                unsigned int    m_nThreadID;
                HANDLE          m_hMutex;
                HANDLE          m_hEvRunning;
                bool            m_bQuit;

        };
};

#endif