#ifndef _Z3_TIMER_
#define _Z3_TIMER_

#include "z3_obj.hpp"

namespace Z3 {

        class Timer : public MemoryObject
        {
        public:
                static Timer*   Instance();
                static void     Destroy();

                HANDLE  AddTimer(uint32_t nObjID, uint32_t millseconds, bool bRepeat = false);
                bool    DeleteTimer(uint32_t nObjID, HANDLE hTimer);

        protected:
                Timer();
                ~Timer();

                static void CALLBACK TimerCallBack(PVOID lpParameter, BOOLEAN bTimerExpired);

        private:
                static Timer*   m_pInstance;
                static HANDLE   m_hMutex;

                static HANDLE   m_hTimerQueue;
        };
};
#endif 
