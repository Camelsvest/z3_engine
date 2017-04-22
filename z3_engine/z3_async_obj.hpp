#ifndef _Z3_ASYNC_OBJ_HPP_
#define _Z3_ASYNC_OBJ_HPP_

#include <Windows.h>
#include "z3_ref_obj.hpp"

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
        
        class AsyncObj : public RefObj
        {
        public:
                AsyncObj(uint32_t nObjID = INVALID_OBJ_ID);

                bool     Lock()  { return m_Lock.On(); }
                void     Unlock()       { return m_Lock.Off(); }

                uint32_t GetObjID()     { return m_nObjID; }

                uint32_t        GetRefCount();
                uint32_t        AddRef();
                void            Release();

        protected:
                virtual ~AsyncObj();

        private:
                Z3::Lock        m_Lock;
                uint32_t        m_nObjID;
        };
};

#endif