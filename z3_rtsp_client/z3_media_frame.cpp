#include "z3_common.hpp"
#include "z3_media_frame.hpp"
#include "z3_rtsp_def.hpp"

using namespace Z3;

MediaFrame::MediaFrame(uint32_t nObjID)
        : Msg(nObjID)
{
}

MediaFrame::~MediaFrame()
{
}

unsigned int MediaFrame::ProtoID()
{
        return MEDIA_FRAME_ID;
}

bool MediaFrame::ToStringImpl(char **ppbuf, unsigned int *pnSize)
{
        TRACE_ERROR("This function is useless in object MediaFrame\r\n");
        return false;
}