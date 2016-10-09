#include "z3_common.hpp"
#include "z3_session_owner.hpp"

using namespace Z3;

SessionOwner::SessionOwner(uint32_t nObjID)
        : AsyncObj(nObjID)
{
}

SessionOwner::~SessionOwner()
{
}