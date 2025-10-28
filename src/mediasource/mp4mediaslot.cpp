
#include "videotrack.h"
#include "audiotrack.h"
#include "mp4mediaslot.h"
#include <hfilename.h>
#include <hlog.h>
#include <htodoholder.h>

extern "C" {
#include <libavformat/avformat.h>
}


namespace {

void setup_media_packet_common(MediaPacket& mp, AVPacket* packet) {

    mp.SetStreamIndex(packet->stream_index);
    mp.SetPts(packet->pts);
    mp.SetDts(packet->dts);
    mp.SetDuration(packet->duration);
    mp.SetPos(packet->pos);
    mp.SetFfmFlags(packet->flags);    

}

}


Mp4MediaSlot::Mp4MediaSlot(HCSTRR strFileName) noexcept
    : slot_base_t(HFileName(strFileName).JustFileName()),
    dev_base_t(strFileName),
    schedule_base_t(),
    m_sps(), m_pps(),
    m_task_id(0), m_sleep_time({0, 0}) {

}


Mp4MediaSlot::~Mp4MediaSlot() {

    if (m_task_id != 0) {
        LOG_WARNING("Mp4MediaSlot UnscheduleDelayedTask task[%d]", m_task_id);
        Schedule().UnscheduleDelayedTask(m_task_id);
    }

}


HRET Mp4MediaSlot::Init() {

    HNOTOK_MSG_RETURN(dev_base_t::Init(), "ffmpeg dev init failed");

    setup_schedule();

    handleBeginPlay();

    HRETURN_OK;

}


HRET Mp4MediaSlot::InComing(HPTR ptr) {

    HCPTR fmt_ptr = getFmtCtx();
    HASSERT_MSG_RETURN(fmt_ptr != nullptr, "fmt pointer is invalid", ERR_STATUS);
    
    AVPacket* packet = static_cast<AVPacket*>(ptr);
    HASSERT_MSG_RETURN(packet != nullptr, "avpacket is invalid", INVL_PARA);

    if (packet->stream_index == getVideoIndex()) {
        return do_incoming_video_packet(ptr);
    }

    if (packet->stream_index == getAudioIndex()) {
        return do_incoming_audio_packet(ptr);
    }

    HRETURN_OK;

}


void Mp4MediaSlot::pull_packet() {

    RET_T ret = dev_base_t::GetPacket();

    IF_NOTOK(ret) {

        if (ret == ERR_NO(OVER_FLOW)) {
            LOG_WARNING("ffmpeg dev seek the end of file. TERMINAL");
            handleTerminal();
            return ;
        }

        handleError();
        LOG_ERROR("ffmpeg dev get packet failed");
        return;

    }

}


void Mp4MediaSlot::onFmtExtraData(const uint8_t* cdata, int len) {
    
    setup_sps_pps_avcc(cdata, len);

    HCPTR ptr = getFmtCtx();
    if (ptr == nullptr) {
        return ;
    }

    const AVFormatContext* cfmt_ctx = static_cast<const AVFormatContext*>(ptr);

    StreamInfo si;
    si.SetName(GetMediaName());
    si.SetStartTime(cfmt_ctx->start_time);
    si.SetDuration(cfmt_ctx->duration);
    si.SetBitRate(cfmt_ctx->bit_rate);
    si.SetStartRealTime(cfmt_ctx->start_time_realtime);

    std::vector<TrackBase*> tracks;
    for (unsigned int i = 0; i < cfmt_ctx->nb_streams; ++i) {
        const AVStream* cstream = cfmt_ctx->streams[i];
        // avoid codec parameters is null.
        if (cstream->codecpar == nullptr) {
            continue;
        }

        if (cstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // generate video track.
            // TODO： for now, we only support H264
            TrackBase* tb = nullptr;
            if (cstream->codecpar->codec_id == AV_CODEC_ID_H264) {
                H264VideoTrack* vt = new H264VideoTrack;
                IF_NOTOK(vt->SetupSpsAndPps(m_sps, m_pps)) {
                    HDELP(vt);
                    return;
                }
                tb = vt;
                
                tb->SetCodecId(MediaCodecId::H264);
            }

            if (tb != nullptr) {
                tb->SetTrackId(cstream->index);
                tb->SetTrackType(TRACK_TYPE::VIDEO);
                tb->SetCodecModuleId(MediaCodecModuleId::DEFAULT);
                tb->SetBitRate(cstream->codecpar->bit_rate);
                tb->SetFrequency(90000);
                tb->SetFormat(cstream->codecpar->format);
                tb->SetProfile(cstream->codecpar->profile);
                tb->SetProfileLevel(cstream->codecpar->level);
                tb->SetTimeBase(cstream->time_base.num, cstream->time_base.den);
                tracks.push_back(tb);
            }
        } 

        if (cstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            // generate audio track.
            AudioTrack* atk = new AudioTrack;
            // TODO: default as aac.
            atk->SetCodecId(MediaCodecId::Aac);

            atk->SetTrackId(cstream->index);
            atk->SetTrackType(TRACK_TYPE::AUDIO);
            atk->SetCodecModuleId(MediaCodecModuleId::DEFAULT);
            atk->SetBitRate(cstream->codecpar->bit_rate);
            atk->SetFrequency(cstream->codecpar->sample_rate);
            atk->SetFormat(cstream->codecpar->format);
            atk->SetProfile(cstream->codecpar->profile);
            atk->SetProfileLevel(cstream->codecpar->level);
            atk->SetTimeBase(cstream->time_base.num, cstream->time_base.den);

            // audio only.
            atk->SetChannelNumber(cstream->codecpar->channels);
            atk->SetSampleRate(cstream->codecpar->sample_rate);
            atk->SetBlockAlign(cstream->codecpar->block_align);
            atk->SetFrameSize(cstream->codecpar->frame_size);
            atk->SetInitialPadding(cstream->codecpar->initial_padding);
            atk->SetTrailingPadding(cstream->codecpar->trailing_padding);

            tracks.push_back(atk);
        }
    }

    handleInited(si, tracks);
}


void Mp4MediaSlot::onBsfExtraData(const uint8_t* data, int len) {

    if (len < 10) {
        return ;
    }

    static unsigned char StartKey4[] = {0x00, 0x00, 0x0, 0x01};
    const uint8_t* pos0 = static_cast<const uint8_t*>(memmem(data, len, StartKey4, 4));
    if (pos0 != nullptr) {

        const uint8_t* pos1 = static_cast<const uint8_t*>(memmem(data + 4, len - 4, StartKey4, 4));
        if (pos1 != nullptr) {

            const uint8_t* sps_data = pos0 + 4;
            int sps_len = pos1 - pos0 - 4;
            
            m_sps.assign(sps_data, sps_data + sps_len);
            //SetSps(sps_data, sps_len);

            const uint8_t* pps_data = pos1 + 4;
            int pps_len = len - (pos1 - data) - 4;
            m_pps.assign(pps_data, pps_data + pps_len);
            //SetPps(pps_data, pps_len);
            //LOG_NORMAL("update extra length[%d] spslen[%d] ppslen[%d]", len, sps_len, pps_len);
        }
    }

}


HRET Mp4MediaSlot::do_incoming_video_packet(HPTR ptr) {
    
    HCPTR fmt_ptr = getFmtCtx();
    const AVFormatContext* fmt_ctx = static_cast<const AVFormatContext*>(fmt_ptr);
    const AVStream* cstream = fmt_ctx->streams[getVideoIndex()];
    AVPacket* packet = static_cast<AVPacket*>(ptr);

    MediaPacket mp;
    mp.SetIndex(getSequence());

    setup_media_packet_common(mp, packet);
    mp.SetTimeBase(cstream->time_base.num, cstream->time_base.den);

    mp.SetRtpPts(av_rescale_q(packet->pts, cstream->time_base, 
        (AVRational){1, 90000}));
    int64_t dura = static_cast<int64_t>(static_cast<double>(packet->duration * 1000) 
        * av_q2d(cstream->time_base)) * 1000;
    mp.SetRealDuration(dura);
    mp.SetTrackType(TRACK_TYPE::VIDEO);
    
    m_sleep_time.v_needs += dura;

    if (packet->flags & AV_PKT_FLAG_KEY) {
        // notify sps nalu packet
        mp.SetNaluType(NaluType::SPS);
        mp.SetData(m_sps.data(), HTO_INT(m_sps.size()));
        handleStreamPacket(mp);
        // notify pps nalu packet
        mp.SetNaluType(NaluType::PPS);
        mp.SetData(m_pps.data(), HTO_INT(m_pps.size()));
        handleStreamPacket(mp);

        mp.SetNaluType(NaluType::IDR_SLICE);
    } else {
        mp.SetNaluType(NaluType::Slice);
    }

    mp.SetData(packet->data, packet->size);
    handleStreamPacket(mp);

    setup_schedule();

    HRETURN_OK;
}


HRET Mp4MediaSlot::do_incoming_audio_packet(HPTR ptr) {

    HCPTR fmt_ptr = getFmtCtx();
    const AVFormatContext* fmt_ctx = static_cast<const AVFormatContext*>(fmt_ptr);
    const AVStream* cstream = fmt_ctx->streams[getAudioIndex()];
    AVPacket* packet = static_cast<AVPacket*>(ptr);

    MediaPacket mp;
    mp.SetIndex(getSequence());

    setup_media_packet_common(mp, packet);
    mp.SetTimeBase(cstream->time_base.num, cstream->time_base.den);

    mp.SetRtpPts(av_rescale_q(packet->pts, cstream->time_base, 
        (AVRational){1, cstream->codecpar->sample_rate}));
    int64_t dura = static_cast<int64_t>(static_cast<double>(packet->duration * 1000) 
        * av_q2d(cstream->time_base)) * 1000;
    m_sleep_time.a_needs += dura;

    mp.SetRealDuration(dura);
    mp.SetTrackType(TRACK_TYPE::AUDIO);
    
    mp.SetNaluType(NaluType::NotNulu);
    
    mp.SetData(packet->data, packet->size);
    handleStreamPacket(mp);

    setup_schedule();
    
    HRETURN_OK;

}


void Mp4MediaSlot::setup_sps_pps_avcc(const uint8_t* data, int) {

    if (data[0] != 1) {
        LOG_WARNING("unexpected extradata version");
        return;
    }

    int iNumOfSps = data[5] & 0x1F;
    if (iNumOfSps != 1) {
        LOG_WARNING("unexpected! number of sps is: %d, it should be 1.", iNumOfSps);
        return ;
    }

    int offset = 6;
    uint16_t sps_length = data[offset] << 8 | data[offset + 1];
    offset += 2;
    const uint8_t* sps_data = data + offset;
    m_sps.assign(sps_data, sps_data + sps_length);
    //SetSps(sps_data, sps_length);
    offset += sps_length;


    int iNumOfPps = data[offset] & 0x1F;
    if (iNumOfPps != 1) {
        LOG_WARNING("unexpected! number of pps is: %d, it should be 1.", iNumOfPps);
        return ;
    }
    offset += 1;
    uint16_t pps_length = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    const uint8_t *pps_data = data + offset;
    m_pps.assign(pps_data, pps_data + pps_length);
    //SetPps(pps_data, pps_length); 

    //LOG_NORMAL("onFmtExtraData len[%d] spslen[%d] ppslen[%d]", len, sps_length, pps_length);
}


void Mp4MediaSlot::setup_schedule() {

    HLL needs = hmin(m_sleep_time.v_needs, m_sleep_time.a_needs);
    if (needs < 0) {
        needs = 0;
    }

    //LOG_NORMAL("needs: [%lld] v[%lld] a[%lld]", needs, m_sleep_time.v_needs,
    //    m_sleep_time.a_needs);
    m_sleep_time.v_needs -= needs;
    if (m_sleep_time.v_needs < 0) {
        m_sleep_time.v_needs = 0;
    } 

    m_sleep_time.a_needs -= needs;
    if (m_sleep_time.a_needs < 0) {
        m_sleep_time.a_needs = 0;
    } 

    
    m_task_id = Schedule().ScheduleDelayedTask(needs, std::bind(&Mp4MediaSlot::pull_packet, this));

}


