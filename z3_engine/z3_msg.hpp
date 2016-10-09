#ifndef _Z3_MSG_HPP_
#define _Z3_MSG_HPP_

#include "z3_obj.hpp"

namespace Z3 {

        class Msg : public AsyncObj
        {
        public:
                Msg(uint32_t nObjID = INVALID_OBJ_ID);

                virtual unsigned int    ProtoID() = 0;
                char*                   ToString(unsigned int *pnSize);

                inline char *           GetBuffer() { return m_pBuffer; }
                inline unsigned int     GetBufSize() { return m_nBufSize; }
                inline void             SetBufSize(unsigned int nSize) { m_nBufSize = nSize; }

        protected:
                virtual ~Msg();

                virtual bool            ToStringImpl(char **ppbuf, unsigned int *pnSize) = 0;

        private:
                char            *m_pBuffer;
                unsigned int    m_nBufSize;
        };        

};

#endif
