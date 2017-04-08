#ifndef _Z3_ASYNC_QUEUE_HPP_
#define _Z3_ASYNC_QUEUE_HPP_

namespace Z3 {

        class AsyncQueue
        {
        public:
                AsyncQueue();
                virtual ~AsyncQueue();

                void    Push(ev_id_t evID, void *pData);
                void*   Pop(ev_id_t &evID);

                bool    Signal(bool bOK = true);
                bool    WaitForEV(Z3EV &ev, uint32_t nTimeout = INFINITE /* millseconds*/);

        protected:


        private:
                std::queue<Z3EV>        m_Queue;
                HANDLE                  m_hEvent;
                HANDLE                  m_hMutex;
        };
};

#endif