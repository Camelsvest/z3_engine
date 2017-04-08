#ifndef _Z3_REF_OBJ_HPP_
#define _Z3_REF_OBJ_HPP_

#include <stdint.h>
#include "z3_memory_obj.hpp"

#define Z3_OBJ_ADDREF(p)        if (p) { p->AddRef(); }
#define Z3_OBJ_RELEASE(p)       if (p) { p->Release(); p = NULL; } 

namespace Z3 {

        class RefObj : public MemoryObject
        {
        public:
                RefObj();

                uint32_t GetRefCount();

                uint32_t AddRef();
                void     Release(bool bFree = true);

        protected:
                virtual ~RefObj();

        private:
                int32_t         m_nRefCount;
        };

};

#endif
