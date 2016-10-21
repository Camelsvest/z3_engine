#include "z3_common.hpp"
#include "z3_msg.hpp"

using namespace Z3;

Msg::Msg(uint32_t nObjID)
        : AsyncObj(nObjID)
{
}

Msg::~Msg()
{
}

bool Msg::ToString(char **ppbuf, unsigned int *pnSize)
{
        TRACE_ERROR("You should override %s in derived class if you want to convert msg to a string.\r\n", __FUNCTION__);

        return false;
}