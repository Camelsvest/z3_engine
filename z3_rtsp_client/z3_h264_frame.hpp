#ifndef _Z3_H264_FRAME_HPP_
#define _Z3_H264_FRAME_HPP_

#include "z3_media_frame.hpp"

namespace Z3 {

        class H264Frame : public MediaFrame
        {
        public:
                H264Frame();

                bool    Allocate(unsigned long lMaxSize);

                virtual unsigned long   GetFrameSize();
                virtual unsigned int    GetFrameType();
                virtual const char *    GetFrameBuffer();

                bool                    AddNalStartCode();
                unsigned long           FillData(const char *pData, size_t nDataSize);

                inline void             SetTimestamp(unsigned long lTimestamp) { m_lTimestamp = lTimestamp; }
                inline unsigned long    GetTimestamp(void) { return m_lTimestamp; }

                void                    Reset();

        protected:
                virtual ~H264Frame();

        private:
                char *          m_pFrameBuf;
                unsigned long   m_lCurrPos;
                unsigned long   m_lMaxFrameSize;

                unsigned long   m_lTimestamp;
        };

};

#endif