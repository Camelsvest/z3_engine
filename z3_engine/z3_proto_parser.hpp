#ifndef _Z3_PROTO_PARSER_HPP_
#define _Z3_PROTO_PARSER_HPP_

#include "z3_obj.hpp"
#include "z3_msg.hpp"

namespace Z3 {

        class ProtoParser : public AsyncObj
        {
        public:
                ProtoParser(uint32_t nObjID = INVALID_OBJ_ID);

                virtual int Parse(const char *pBuf, unsigned int nLen, void *pData, Msg **pMsg /* output */) = 0;

        protected:
                virtual ~ProtoParser();
        };

};

#endif