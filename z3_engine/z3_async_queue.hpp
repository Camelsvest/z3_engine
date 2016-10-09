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
                void*   WaitForEV(ev_id_t &evID, uint32_t nTimeout = 20 /* millseconds*/);

        protected:


        private:
                std::queue<Z3EV>        m_Queue;
                HANDLE                  m_hEvent;
                HANDLE                  m_hMutex;
        };
};

#endif