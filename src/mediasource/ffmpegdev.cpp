

#include "ffmpegdev.h"
#include <hlog.h>
#include <htodoholder.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
//#include <libavcodec/bsf.h>
}
#include "streaminfo.h"

class ffmpeg_setting {
public:
    ffmpeg_setting() noexcept {
        //avdevice_register_all();
        avformat_network_init();
    }
};

static const ffmpeg_setting g_settings;

FfmpegMediaDev::FfmpegMediaDev(HCSTRR strName) noexcept
    : base_class_t("FfmpegMediaDev"),
    m_strName(strName),
    m_fmt(nullptr), m_bsf_ctx(nullptr),
    m_src_pkt(nullptr), m_out_pkt(nullptr),
    m_video_index(-1), m_audio_index(-1), m_sequence(0) {

}


FfmpegMediaDev::~FfmpegMediaDev() {

    uninit();

}


HRET FfmpegMediaDev::Init() {

    LOG_NORMAL("openfile [%s]", m_strName.c_str());

    AVFormatContext* input_ctx = nullptr;
    HASSERT_MSG_RETURN(avformat_open_input(&input_ctx, m_strName.c_str(), nullptr, nullptr) == 0,
        "avformat_open_input failed", DEP_ERROR);

    HASSERT_MSG_RETURN(avformat_find_stream_info(input_ctx, nullptr) >= 0, 
        "avformat_find_stream_info failed", DEP_ERROR);

    m_video_index = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0); 
    HASSERT_MSG_RETURN(m_video_index >= 0, "video index is invalid", DEP_ERROR);

    m_audio_index = av_find_best_stream(input_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    HASSERT_MSG_RETURN(m_audio_index >= 0, "audio index is invalid", DEP_ERROR);

    m_fmt = input_ctx;
    LOG_NORMAL("video index[%d] audio index[%d]", m_video_index, m_audio_index);

    onFmtExtraData(input_ctx->streams[m_video_index]->codecpar->extradata, 
        input_ctx->streams[m_video_index]->codecpar->extradata_size);

    const AVBitStreamFilter* filter = av_bsf_get_by_name("h264_mp4toannexb");
    HASSERT_MSG_RETURN(filter != nullptr, "av_bsf_get_by_name failed", DEP_ERROR);

    AVBSFContext* bsf_ctx = nullptr;
    HASSERT_MSG_RETURN(av_bsf_alloc(filter, &bsf_ctx) >= 0, "av_bsf_alloc failed", DEP_ERROR);

    HASSERT_MSG_RETURN(avcodec_parameters_copy(bsf_ctx->par_in, 
        input_ctx->streams[m_video_index]->codecpar) >= 0, "avcodec_parameters_copy failed",
        DEP_ERROR);

    HASSERT_MSG_RETURN(av_bsf_init(bsf_ctx) >= 0, "av_bsf_init failed", DEP_ERROR);
    
    m_bsf_ctx = bsf_ctx;

    AVPacket* src_pkt = av_packet_alloc();
    AVPacket* out_pkt = av_packet_alloc();

    m_src_pkt = src_pkt;
    m_out_pkt = out_pkt;

    HRETURN_OK;

}


HRET FfmpegMediaDev::GetPacket() {

    AVFormatContext* input_ctx = static_cast<AVFormatContext*>(m_fmt);
    AVBSFContext* bsf_ctx = static_cast<AVBSFContext*>(m_bsf_ctx);
    AVPacket* src_pkt = static_cast<AVPacket*>(m_src_pkt);
    AVPacket* out_pkt = static_cast<AVPacket*>(m_out_pkt);

    HASSERT_MSG_RETURN(av_read_frame(input_ctx, src_pkt) >= 0, "read ffmpeg failed", OVER_FLOW);

    HSCOPE_EXIT { 
        av_packet_unref(src_pkt); 
        ++m_sequence;
    };

    if (src_pkt->stream_index == m_video_index) {

        HASSERT_MSG_RETURN(av_bsf_send_packet(bsf_ctx, src_pkt) >= 0,
            "send bsf failed", DEP_ERROR);

        while (av_bsf_receive_packet(bsf_ctx, out_pkt) == 0) {
            break;
        }

        HSCOPE_EXIT { av_packet_unref(out_pkt); };

        if (bsf_ctx->par_out->extradata_size > 0 and bsf_ctx->par_out->extradata != nullptr) {
            onBsfExtraData(bsf_ctx->par_out->extradata, bsf_ctx->par_out->extradata_size);
        }

        return InComing(out_pkt);

    } else {

        return InComing(src_pkt);

    }

    HRETURN_OK;

}


void FfmpegMediaDev::uninit() {

    // 释放ffmpeg 
    if (m_src_pkt != nullptr) {
        AVPacket* src_pkt = static_cast<AVPacket*>(m_src_pkt);
        av_packet_free(&src_pkt);
        m_src_pkt = nullptr;
    }

    if (m_out_pkt != nullptr) {
        AVPacket* out_pkt = static_cast<AVPacket*>(m_out_pkt);
        av_packet_free(&out_pkt);
        m_out_pkt = nullptr;
    }

    if (m_bsf_ctx != nullptr) {
        AVBSFContext* bsf_ctx = static_cast<AVBSFContext*>(m_bsf_ctx);
        av_bsf_free(&bsf_ctx);
        m_fmt = nullptr;
    }

    if (m_fmt != nullptr) {
        AVFormatContext* input_ctx = static_cast<AVFormatContext*>(m_fmt);
        avformat_close_input(&input_ctx);
        m_fmt = nullptr;
    }

}



