#include "stdafx.h"
#include "z3_common.hpp"
#include "z3_connection.h"

using namespace Z3;

Connection::Connection(HANDLE hIOCP)
        : Connector(hIOCP, INVALID_OBJ_ID)
{

}

Connection::~Connection()
{

}

ProtoParser*  Connection::GetProtoParser()
{
        return NULL;
}

int Connection::Dispatch(Msg *pMsg, void *pData)
{
        return 0;
}

int Connection::OnConnect(uint32_t nErrorCode, bool bExpired)
{
        return Connector::OnConnect(nErrorCode, bExpired);
}