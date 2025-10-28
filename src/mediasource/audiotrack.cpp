

#include "audiotrack.h"
#include <sstream>

#include <hlog.h>

namespace {

static HN const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

static HN get_frequency_index(HN frequency) {

    for (size_t i = 0; i < sizeof(samplingFrequencyTable) / sizeof(HN); ++i) {

        if (samplingFrequencyTable[i] == frequency) {
            return i;
        }

    }

    return -1;

}

}


AudioTrack::AudioTrack() noexcept
    : track_base_t(), 
    m_channel_number(0), m_sample_rate(0),
    m_block_align(0), m_frame_size(0),
    m_initial_padding(0), m_trailing_padding(0) {

}


AudioTrack::~ AudioTrack() noexcept {

}



HSTR AudioTrack::GenerateSDP() const {

    //return HSTR("a=fmtp:97 streamtype=5;profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=1310\r\n");

    //HN profile = GetProfile();
    // AAC profile
    HN profile = 1;
    HN frequency_index = get_frequency_index(GetTimebase().den);
    HN channel_num = GetChannelNumber();

    unsigned char audioSpecificConfig[2];
    u_int8_t const audioObjectType = profile + 1;

    audioSpecificConfig[0] = (audioObjectType<<3) | (frequency_index>>1);
    audioSpecificConfig[1] = (frequency_index<<7) | (channel_num<<3);

    HSTR strConfig = HStr::Format("%02X%02X", audioSpecificConfig[0], audioSpecificConfig[1]);

    //LOG_NORMAL("timebase den[%d] frequency[%d] channel[%d] profile[%d] config[%s]", 
    //    GetTimebase().den, GetFrequency(), GetChannelNumber(), GetProfile(),
    //    strConfig.c_str());
    

    std::stringstream ss;
    ss << "m=audio 0 RTP/AVP 97\r\n"
        << "c=IN IP4 0.0.0.0\r\n"
        << "b=AS:96\r\n"
        << "a=rtpmap:97 MPEG4-GENERIC/" << GetTimebase().den << "/" << GetChannelNumber() << "\r\n"
        << "a=fmtp:97 streamtype=5;profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=" << strConfig << "\r\n"
        << "a=control:track2\r\n";
        
    return ss.str();

}

