

#ifndef __H_RTSPPARSER_H__
#define __H_RTSPPARSER_H__

#include <huicpp.h>
#include "../rtprtcp/rtpinfo.h"

using namespace HUICPP;

namespace rtsp_parser {

HN find_rtsp_header_pos(HCSZ data, HN data_len);

bool parse_rtsp_request_string(HCSZ header, HN header_len, HSTRR strCmd, HSTRR urlPreSuffix,
    HSTRR strUrlSuffix, HSTRR strCseq, HSTRR strSessionIdStr, HNR content_length);

bool parse_setup_rtp_ports(HCSTRR strTransport, HUNR port0, HUNR port1);

bool parse_setup_rtp_channels(HCSTRR strTransport, HUNR channel0, HUNR channel1);

}



#endif // __H_RTSPPARSER_H__