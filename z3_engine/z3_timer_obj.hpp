#ifndef _Z3_TIMER_OBJ_HPP_
#define _Z3_TIMER_OBJ_HPP_

#include <map>
#include "z3_async_obj.hpp"

#define Z3_INVALID_TIMER_ID     0
#define Z3_INVALID_TIMER_HANDLE 0

namespace Z3 {

        /*
        * Description: 全局Timer对象，负责所有timer的创建和删除
        */
        class TimerEngine : public RefObj
        {
        public:
                static TimerEngine*     Instance();
                static void             Destroy();
                static uint32_t         CreateTimerID();

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
                static uint32_t         m_nTimerIDIndex;
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

#endif
