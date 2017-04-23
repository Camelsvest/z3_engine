#pragma once

#include "z3_client.hpp"
#include "z3_connection.h"

namespace Z3 {

        class TestClient : public Client
        {
        public:
                TestClient();

                Connection* AddConnection(const char *pszHost, uint16_t port);
                virtual void    OnNotify(ev_id_t evID, uint32_t nErrorCode, void *pData);

        protected:
                virtual ~TestClient();

                virtual void    OnClientStart(void);
                virtual void    OnClientStop(void);
        };
};