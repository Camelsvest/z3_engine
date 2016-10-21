#ifndef _Z3_MEDIA_FRAME_HPP_
#define _Z3_MEDIA_FRAME_HPP_

#include "z3_msg.hpp"

namespace Z3 {

        enum _FRAME_TYPE
        {
                UNKNOWN_VIDEO_FRAME = 0,
                H264_VIDEO_FRAME,
                MPEG4_VIDEO_FRAME,

                UNKNOWN_AUDIO_FRAME = 0xFF,
                G711A_AUDIO_FRAME,
                G711U_AUDIO_FRAME
        };

        class MediaFrame : public Msg
        {
        public:
                MediaFrame(uint32_t nObjID = INVALID_OBJ_ID);

                virtual unsigned int    ProtoID();

                virtual unsigned long   GetFrameSize() = 0;
                virtual unsigned int    GetFrameType() = 0;
                virtual const char *    GetFrameBuffer() = 0;

        protected:
                virtual ~MediaFrame();
        };


};

#endif