#pragma once

#include "z3_connector.hpp"

namespace Z3 {

        class Connection : public Connector
        {
        public:
                Connection(HANDLE hIOCP);

        protected:
                virtual ~Connection();

                virtual int     OnConnect(uint32_t nErrorCode);

                virtual ProtoParser*    GetProtoParser();
                virtual int     Dispatch(Msg *pMsg, void *pData);
        };
}
