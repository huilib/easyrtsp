

#include "rtspparser.h"

#include <hstr.h>

namespace {
static void decodeURL(HSTRR str) {

    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    memcpy(buffer, str.c_str(), str.length());
    char* url = buffer;

    // Replace (in place) any %<hex><hex> sequences with the appropriate 8-bit character.
    char* cursor = url;
    while (*cursor) {
        if ((cursor[0] == '%') &&
            cursor[1] && isxdigit(cursor[1]) &&
            cursor[2] && isxdigit(cursor[2])) {
            // We saw a % followed by 2 hex digits, so we copy the literal hex value into the URL, then advance the cursor past it:
            char hex[3];
            hex[0] = cursor[1];
            hex[1] = cursor[2];
            hex[2] = '\0';
            *url++ = (char)strtol(hex, NULL, 16);
            cursor += 3;
        } else {
            // Common case: This is a normal character or a bogus % expression, so just copy it
            *url++ = *cursor++;
        }
    }
    
    *url = '\0';

    str.clear();
    str.assign(buffer);
}
}

namespace rtsp_parser {

HN find_rtsp_header_pos(HCSZ data, HN data_len) {

    if (data == nullptr or data_len < 3) {
        return -1;
    }

    HCSZ pos = data, lastCrtl = pos;
    while (pos < &data[data_len-1]) {
        if (*pos == '\r' && *(pos+1) == '\n') {
            if (pos - lastCrtl == 2) { // This is it:
                break;
            }
            lastCrtl = pos;
        }
        ++pos;
    }

    return pos - data;
}


bool parse_rtsp_request_string(HCSZ header, HN header_len, HSTRR strCmd, HSTRR urlPreSuffix,
    HSTRR strUrlSuffix, HSTRR strCseq, HSTRR strSessionIdStr, HNR content_length) {

    strCmd.clear();
    urlPreSuffix.clear();
    strUrlSuffix.clear();
    strCseq.clear();
    strSessionIdStr.clear();
    content_length = 0;

    // "Be liberal in what you accept": 
    //      Skip over any whitespace at the start of the request:
    HN i;
    for (i = 0; i < header_len; ++i) {
        char c = header[i];
        if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\0')) break;
    }
    if (i == header_len) {
        // The request consisted of nothing but whitespace!
        return false; 
    }

    bool res = false;
    // Then read everything up to the next space (or tab) as the command name:
    for (; i < header_len; ++i) {
        char c = header[i];
        if (c == ' ' || c == '\t') {
            res = true;
            break;
        }
        strCmd.push_back(c);
        //resultCmdName[i1] = c;
    }
    //resultCmdName[i1] = '\0';
    if (not res) {
        return res;
    }

    // Skip over the prefix of any "rtsp://" or "rtsp:/" (or "rtsps://" or "rtsps:/")
    // URL that follows:
    HN j = i + 1;
    while (j < header_len and (header[j] == ' ' || header[j] == '\t')) {
        // skip over any additional white space
        ++j; 
    }
    for (; (int)j < (int)(header_len - 8); ++j) {
        if ((header[j] == 'r' || header[j] == 'R')
        && (header[j+1] == 't' || header[j+1] == 'T')
        && (header[j+2] == 's' || header[j+2] == 'S')
        && (header[j+3] == 'p' || header[j+3] == 'P')) {
            /*if (header[j+4] == 's' || header[j+4] == 'S') {
                urlIsRTSPS = True;
                ++j;
            }*/
            if (header[j+4] == ':' && header[j+5] == '/') {
                j += 6;
                if (header[j] == '/') {
                    // This is a "rtsp(s)://" URL; skip over the host:port part that follows:
                    ++j;
                    while (j < header_len && header[j] != '/' && header[j] != ' ') {
                        ++j;
                    }
                } else {
                    // This is a "rtsp(s):/" URL; back up to the "/":
                    --j;
                }
                i = j;
                break;
            }
        }
    }	


    // Look for the URL suffix (before the following "RTSP/"):
    res = false;
    for (HN k = i+1; k < (header_len-5); ++k) {
        if (header[k] == 'R' && header[k+1] == 'T' &&
	        header[k+2] == 'S' && header[k+3] == 'P' && header[k+4] == '/') {
            while (--k >= i && header[k] == ' ') {} // go back over all spaces before "RTSP/"
            HN k1 = k;
            while (k1 > i && header[k1] != '/') --k1;

            // ASSERT: At this point
            //   i: first space or slash after "host" or "host:port"
            //   k: last non-space before "RTSP/"
            //   k1: last slash in the range [i,k]

            // The URL suffix comes from [k1+1,k]
            // Copy "resultURLSuffix":
            HN n = 0, k2 = k1+1;
            (void)n;
            if (k2 <= k) {
                /*if (k - k1 + 1 > resultURLSuffixMaxSize) {
                    // there's no room
                    return false; 
                }*/
                while (k2 <= k) {
                    //resultURLSuffix[n++] = reqStr[k2++];
                    strUrlSuffix.push_back(header[k2++]);
                }
            }
            //resultURLSuffix[n] = '\0';

            // The URL 'pre-suffix' comes from [i+1,k1-1]
            // Copy "resultURLPreSuffix":
            n = 0; k2 = i+1;
            if (k2+1 <= k1) {
                //if (k1 - i > resultURLPreSuffixMaxSize) return False; // there's no room
                while (k2 <= k1-1) {
                    //resultURLPreSuffix[n++] = reqStr[k2++];
                    urlPreSuffix.push_back(header[k2++]);
                }
            }
            //resultURLPreSuffix[n] = '\0';

            //decodeURL(resultURLPreSuffix);
            decodeURL(urlPreSuffix);

            i = k + 7; // to go past " RTSP/"
            res = true;
            break;
        }
    }
    if (not res) {
        return false;
    }

    // Look for "CSeq:" (mandatory, case insensitive), skip whitespace,
    // then read everything up to the next \r or \n as 'CSeq':
    res = false;
    for (j = i; j < (header_len-5); ++j) {
        if (strncasecmp("CSeq:", &header[j], 5) == 0) {
            j += 5;
            while (j < header_len && (header[j] ==  ' ' || header[j] == '\t')) ++j;
            HN n;
            /*for (n = 0; n < resultCSeqMaxSize-1 && j < reqStrSize; ++n,++j) {
                char c = reqStr[j];
                if (c == '\r' || c == '\n') {
                    parseSucceeded = True;
                    break;
                }

                resultCSeq[n] = c;
            }
            resultCSeq[n] = '\0';*/
            for (; j < header_len; ++n,++j) {
                char c = header[j];
                if (c == '\r' || c == '\n') {
                    res = true;
                    break;
                }
                strCseq.push_back(c);
                //resultCSeq[n] = c;
            }
            //resultCSeq[n] = '\0';
            break;
        }
    }
    if (!res) { 
        return false;
    }

    // Look for "Session:" (optional, case insensitive), skip whitespace,
    // then read everything up to the next \r or \n as 'Session':
    //resultSessionIdStr[0] = '\0'; // default value (empty string)
    for (j = i; j < (header_len-8); ++j) {
        if (strncasecmp("Session:", &header[j], 8) == 0) {
            j += 8;
            while (j < header_len && (header[j] ==  ' ' || header[j] == '\t')) ++j;
            /*unsigned n;
            for (n = 0; n < resultSessionIdStrMaxSize-1 && j < header_len; ++n,++j) {
                char c = reqStr[j];
                if (c == '\r' || c == '\n') {
                    break;
                }

                resultSessionIdStr[n] = c;
            }
            resultSessionIdStr[n] = '\0';*/
            for (; j < header_len; ++j) {
                char c = header[j];
                if (c == '\r' || c == '\n') {
                    break;
                }
                strSessionIdStr.push_back(c);
            }
            break;
        }
    }


    // Also: Look for "Content-Length:" (optional, case insensitive)
    content_length = 0; // default value
    for (j = i; (int)j < (int)(header_len-15); ++j) {
        if (strncasecmp("Content-Length:", &(header[j]), 15) == 0) {
            j += 15;
            while (j < header_len && (header[j] ==  ' ' || header[j] == '\t')) ++j;
            unsigned num;
            if (sscanf(&header[j], "%u", &num) == 1) {
                content_length = num;
            }
        }
    }    

    return res;
}


bool parse_setup_rtp_ports(HCSTRR strTransport, HUNR port0, HUNR port1) {

    HVSTR ss;
    HIGNORE_RETURN(HStr(strTransport).Split(";", ss));

    if (ss.size() < 2) {
        return false;
    }

    for(HVSTR::size_type i = 0; i < ss.size(); ++i) {
        HCSTRR str = ss[i];

        HSTR::size_type pos = str.find("client_port=");
        if (pos == HSTR::npos) {
            continue;
        }

        HVSTR pps;
        IF_NOTOK(HStr(str).Split("=", pps)) {
            return false;
        }
        if (pps.size() != 2) {
            return false;
        }

        HVSTR sps;
        IF_NOTOK(HStr(pps[1]).Split("-", sps)) {
            return false;
        }

        port0 = HStr(sps[0]).ToN();
        port1 = HStr(sps[1]).ToN();

        return true;
    }

    return false;
}


bool parse_setup_rtp_channels(HCSTRR strTransport, HUNR channel0, HUNR channel1) {

    HVSTR ss;
    HIGNORE_RETURN(HStr(strTransport).Split("interleaved=", ss));

    if (ss.size() < 2) {
        return false;
    }

    HVSTR sps;
    IF_NOTOK(HStr(ss[1]).Split("-", sps)) {
        return false;
    }

    if (sps.size() != 2) {
        return false;
    }

    channel0 = HStr(sps[0]).ToN();
    channel1 = HStr(sps[1]).ToN();

    return true;

}

}