#ifndef _Z3_MSG_HPP_
#define _Z3_MSG_HPP_

#include "z3_obj.hpp"

namespace Z3 {

        class Msg : public AsyncObj
        {
        public:
                Msg(uint32_t nObjID = INVALID_OBJ_ID);

                virtual unsigned int    ProtoID() = 0;
                virtual bool            ToString(char **ppbuf, unsigned int *pnSize);

        protected:
                virtual ~Msg();
        };        

};

#endif
