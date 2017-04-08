#ifndef _Z3_SESSION_OWNER_HPP_
#define _Z3_SESSION_OWNER_HPP_

#include "z3_async_obj.hpp"

namespace Z3 {

        class SessionOwner : public AsyncObj
        {
        public:
                SessionOwner(uint32_t nObjID = INVALID_OBJ_ID);

                virtual void OnNotify(uint32_t nSessionState, void *pData) = 0;

        protected:
                virtual ~SessionOwner();
        };
};
#endif