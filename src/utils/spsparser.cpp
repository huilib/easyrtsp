

#include "spsparser.h"
#include "bitreader.h"

#ifdef SPS_DEBUG
#include <iostream>
void DEBUG_SPS(HCSTRR str) {
    std::cout << str << std::endl;
}
#else 
#define DEBUG_SPS(str)
#endif 

namespace {

static constexpr const int MAX_SPS_COUNT = 32;
static constexpr const int MIN_LOG2_MAX_FRAME_NUM = 4; 
static constexpr const int MAX_LOG2_MAX_FRAME_NUM = (12 + 4);

static const uint8_t default_scaling4[2][16] = {
    {  6, 13, 20, 28, 13, 20, 28, 32,
      20, 28, 32, 37, 28, 32, 37, 42 },
    { 10, 14, 20, 24, 14, 20, 24, 27,
      20, 24, 27, 30, 24, 27, 30, 34 }
};

static const uint8_t default_scaling8[2][64] = {
    {  6, 10, 13, 16, 18, 23, 25, 27,
      10, 11, 16, 18, 23, 25, 27, 29,
      13, 16, 18, 23, 25, 27, 29, 31,
      16, 18, 23, 25, 27, 29, 31, 33,
      18, 23, 25, 27, 29, 31, 33, 36,
      23, 25, 27, 29, 31, 33, 36, 38,
      25, 27, 29, 31, 33, 36, 38, 40,
      27, 29, 31, 33, 36, 38, 40, 42 },
    {  9, 13, 15, 17, 19, 21, 22, 24,
      13, 13, 17, 19, 21, 22, 24, 25,
      15, 17, 19, 21, 22, 24, 25, 27,
      17, 19, 21, 22, 24, 25, 27, 28,
      19, 21, 22, 24, 25, 27, 28, 30,
      21, 22, 24, 25, 27, 28, 30, 32,
      22, 24, 25, 27, 28, 30, 32, 33,
      24, 25, 27, 28, 30, 32, 33, 35 }
};

const uint8_t ff_zigzag_direct[64] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};


const uint8_t ff_zigzag_scan[16+1] = {
    0 + 0 * 4, 1 + 0 * 4, 0 + 1 * 4, 0 + 2 * 4,
    1 + 1 * 4, 2 + 0 * 4, 3 + 0 * 4, 2 + 1 * 4,
    1 + 2 * 4, 0 + 3 * 4, 1 + 3 * 4, 2 + 2 * 4,
    3 + 1 * 4, 3 + 2 * 4, 2 + 3 * 4, 3 + 3 * 4,
};

const uint8_t ff_log2_run[41] = {
 0,  0,  0,  0,  1,  1,  1,  1,
 2,  2,  2,  2,  3,  3,  3,  3,
 4,  4,  5,  5,  6,  6,  7,  7,
 8,  9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23,
24,
};

static int decode_scaling_list(BitReader *gb, uint8_t *factors, int size,
                               const uint8_t *jvt_list, const uint8_t *fallback_list,
                               uint16_t *mask, int pos)
{
    int i, last = 8, next = 8;
    const uint8_t *scan = size == 16 ? ff_zigzag_scan : ff_zigzag_direct;
    uint16_t seq_scaling_list_present_flag = gb->getBits1();
    *mask |= (seq_scaling_list_present_flag << pos);
    if (!seq_scaling_list_present_flag) /* matrix not written, we use the predicted one */
        memcpy(factors, fallback_list, size * sizeof(uint8_t));
    else
        for (i = 0; i < size; i++) {
            if (next) {
                int v = gb->get_se_golomb();
                if (v < -128 || v > 127) {
                    return -1;
                }
                next = (last + v) & 0xff;
            }
            if (!i && !next) { /* matrix not written, we use the preset one */
                memcpy(factors, jvt_list, size * sizeof(uint8_t));
                break;
            }
            last = factors[scan[i]] = next ? next : last;
        }
    return 0;
}

static int decode_scaling_matrices(BitReader *gb, const SPS *sps,
                                    int present_flag, uint16_t *mask,
                                    uint8_t(*scaling_matrix4)[16],
                                    uint8_t(*)[64])
{
    int is_sps = 1;
    int fallback_sps = !is_sps && sps->scaling_matrix_present;
    const uint8_t *fallback[4] = {
        fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
        fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
        fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
        fallback_sps ? sps->scaling_matrix8[3] : default_scaling8[1]
    };
    int ret = 0;
    *mask = 0x0;
    if (present_flag) {
        ret |= decode_scaling_list(gb, scaling_matrix4[0], 16, default_scaling4[0], fallback[0], mask, 0);        // Intra, Y
        ret |= decode_scaling_list(gb, scaling_matrix4[1], 16, default_scaling4[0], scaling_matrix4[0], mask, 1); // Intra, Cr
        ret |= decode_scaling_list(gb, scaling_matrix4[2], 16, default_scaling4[0], scaling_matrix4[1], mask, 2); // Intra, Cb
        ret |= decode_scaling_list(gb, scaling_matrix4[3], 16, default_scaling4[1], fallback[1], mask, 3);        // Inter, Y
        ret |= decode_scaling_list(gb, scaling_matrix4[4], 16, default_scaling4[1], scaling_matrix4[3], mask, 4); // Inter, Cr
        ret |= decode_scaling_list(gb, scaling_matrix4[5], 16, default_scaling4[1], scaling_matrix4[4], mask, 5); // Inter, CbW
        if (!ret)
            ret = is_sps;
    }

    return ret;
}

const AVRational ff_h2645_pixel_aspect[] = {
    {   0,  1 },
    {   1,  1 },
    {  12, 11 },
    {  10, 11 },
    {  16, 11 },
    {  40, 33 },
    {  24, 11 },
    {  20, 11 },
    {  32, 11 },
    {  80, 33 },
    {  18, 11 },
    {  15, 11 },
    {  64, 33 },
    { 160, 99 },
    {   4,  3 },
    {   3,  2 },
    {   2,  1 },
};

static const int level_max_dpb_mbs[][2] = {
    { 10, 396       },
    { 11, 900       },
    { 12, 2376      },
    { 13, 2376      },
    { 20, 2376      },
    { 21, 4752      },
    { 22, 8100      },
    { 30, 8100      },
    { 31, 18000     },
    { 32, 20480     },
    { 40, 32768     },
    { 41, 32768     },
    { 42, 34816     },
    { 50, 110400    },
    { 51, 184320    },
    { 52, 184320    },
};


void ff_h2645_decode_common_vui_params(BitReader *gb, H2645VUI *vui)
{

    vui->aspect_ratio_info_present_flag = gb->getBits1();
    if (vui->aspect_ratio_info_present_flag) {
        vui->aspect_ratio_idc = gb->getBits(8);
        if (vui->aspect_ratio_idc < HTO_INT((sizeof(ff_h2645_pixel_aspect) / sizeof((ff_h2645_pixel_aspect)[0])))) {
            vui->sar = ff_h2645_pixel_aspect[vui->aspect_ratio_idc];
        } else if (vui->aspect_ratio_idc == 255) {
            vui->sar.num = gb->getBits(16);
            vui->sar.den = gb->getBits(16);
        } 
    } else {
        vui->sar = (AVRational){ 0, 1 };
    }
    vui->overscan_info_present_flag = gb->getBits1();
    if (vui->overscan_info_present_flag)
        vui->overscan_appropriate_flag = gb->getBits1();

    vui->video_signal_type_present_flag = gb->getBits1();
    if (vui->video_signal_type_present_flag) {
        vui->video_format                    = gb->getBits(3);
        vui->video_full_range_flag           = gb->getBits1();
        vui->colour_description_present_flag = gb->getBits1();
        if (vui->colour_description_present_flag) {
            vui->colour_primaries         = gb->getBits(8);
            vui->transfer_characteristics = gb->getBits(8);
            vui->matrix_coeffs            = gb->getBits(8);
        }
    }

    vui->chroma_loc_info_present_flag = gb->getBits1();
    if (vui->chroma_loc_info_present_flag) {
        vui->chroma_sample_loc_type_top_field    = gb->get_ue_golomb_31();
        vui->chroma_sample_loc_type_bottom_field = gb->get_ue_golomb_31();
        if (vui->chroma_sample_loc_type_top_field <= 5)
            vui->chroma_location = vui->chroma_sample_loc_type_top_field + 1;
        else
            vui->chroma_location = 0;
    } else
        vui->chroma_location = 1;
}


static inline int decode_hrd_parameters(BitReader *gb, SPS *sps)
{
    int cpb_count, i;
    cpb_count = gb->get_ue_golomb_31() + 1;

    if (cpb_count > 32) {
        return -1;
    }

    sps->cpr_flag = 0x0;
    sps->bit_rate_scale = gb->getBits(4);
    gb->getBits(4); /* cpb_size_scale */
    for (i = 0; i < cpb_count; i++) {
        sps->bit_rate_value[i] = gb->get_ue_golomb_long() + 1; /* bit_rate_value_minus1 + 1 */
        sps->cpb_size_value[i] = gb->get_ue_golomb_long() + 1; /* cpb_size_value_minus1 + 1 */
        sps->cpr_flag         |= gb->getBits1() << i;
    }
    sps->initial_cpb_removal_delay_length = gb->getBits(5) + 1;
    sps->cpb_removal_delay_length         = gb->getBits(5) + 1;
    sps->dpb_output_delay_length          = gb->getBits(5) + 1;
    sps->time_offset_length               = gb->getBits(5);
    sps->cpb_cnt                          = cpb_count;
    return 0;
}

static inline int decode_vui_parameters(BitReader *gb, SPS *sps)
{
    ff_h2645_decode_common_vui_params(gb, &sps->vui);

    if (gb->show_bits(1) && gb->get_bits_left() < 10) {
        return 0;
    }

    sps->timing_info_present_flag = gb->getBits1();
    if (sps->timing_info_present_flag) {
        unsigned num_units_in_tick = gb->get_bits_long(32);
        unsigned time_scale        = gb->get_bits_long(32);
        if (!num_units_in_tick || !time_scale) {
            sps->timing_info_present_flag = 0;
        } else {
            sps->num_units_in_tick = num_units_in_tick;
            sps->time_scale = time_scale;
        }
        sps->fixed_frame_rate_flag = gb->getBits1();
    }

    sps->nal_hrd_parameters_present_flag = gb->getBits1();
    if (sps->nal_hrd_parameters_present_flag)
        if (decode_hrd_parameters(gb, sps) < 0)
            return -1;
    sps->vcl_hrd_parameters_present_flag = gb->getBits1();
    if (sps->vcl_hrd_parameters_present_flag)
        if (decode_hrd_parameters(gb, sps) < 0)
            return -1;
    if (sps->nal_hrd_parameters_present_flag ||
        sps->vcl_hrd_parameters_present_flag) {
        gb->getBits1();
    }
    sps->pic_struct_present_flag = gb->getBits1();
    if (!gb->get_bits_left())
        return 0;
    sps->bitstream_restriction_flag = gb->getBits1();
    if (sps->bitstream_restriction_flag) {
        gb->getBits1();    /* motion_vectors_over_pic_boundaries_flag */
        gb->get_ue_golomb_31(); /* max_bytes_per_pic_denom */
        gb->get_ue_golomb_31(); /* max_bits_per_mb_denom */
        gb->get_ue_golomb_31(); /* log2_max_mv_length_horizontal */
        gb->get_ue_golomb_31(); /* log2_max_mv_length_vertical */
        sps->num_reorder_frames = gb->get_ue_golomb_31();
        sps->max_dec_frame_buffering = gb->get_ue_golomb_31();

        if (gb->get_bits_left() < 0) {
            sps->num_reorder_frames         = 0;
            sps->bitstream_restriction_flag = 0;
        }

        if (sps->num_reorder_frames > 16) {
            sps->num_reorder_frames = 16;
            return -1;
        }
    }

    return 0;
}

}


bool ParseSps(HCMEMR mem, SPS sps) {

    BitReader gb(mem.data(), HTO_INT(mem.size()));

    int profile_idc, level_idc, constraint_set_flags = 0;
    int sps_id = 0;
    int i = 0, log2_max_frame_num_minus4 = 0, ret = -1;

    profile_idc           = gb.getBits(8);
    constraint_set_flags |= gb.getBits1() << 0;   // constraint_set0_flag
    constraint_set_flags |= gb.getBits1() << 1;   // constraint_set1_flag
    constraint_set_flags |= gb.getBits1() << 2;   // constraint_set2_flag
    constraint_set_flags |= gb.getBits1() << 3;   // constraint_set3_flag
    constraint_set_flags |= gb.getBits1() << 4;   // constraint_set4_flag
    constraint_set_flags |= gb.getBits1() << 5;   // constraint_set5_flag
    gb.skip_bits(2);                              // reserved_zero_2bits
    level_idc = gb.getBits(8);
    sps_id    = gb.get_ue_golomb_31();

    if (sps_id >= MAX_SPS_COUNT) {
        DEBUG_SPS("sps_id is invalid");
        return false;
    }

    sps.sps_id               = sps_id;
    sps.time_offset_length   = 24;
    sps.profile_idc          = profile_idc;
    sps.constraint_set_flags = constraint_set_flags;
    sps.level_idc            = level_idc;
    sps.vui.video_full_range_flag = -1;

    memset(sps.scaling_matrix4, 16, sizeof(sps.scaling_matrix4));
    memset(sps.scaling_matrix8, 16, sizeof(sps.scaling_matrix8));
    sps.scaling_matrix_present = 0;
    sps.vui.matrix_coeffs = 2;

    if (sps.profile_idc == 100 ||  // High profile
        sps.profile_idc == 110 ||  // High10 profile
        sps.profile_idc == 122 ||  // High422 profile
        sps.profile_idc == 244 ||  // High444 Predictive profile
        sps.profile_idc ==  44 ||  // Cavlc444 profile
        sps.profile_idc ==  83 ||  // Scalable Constrained High profile (SVC)
        sps.profile_idc ==  86 ||  // Scalable High Intra profile (SVC)
        sps.profile_idc == 118 ||  // Stereo High profile (MVC)
        sps.profile_idc == 128 ||  // Multiview High profile (MVC)
        sps.profile_idc == 138 ||  // Multiview Depth High profile (MVCD)
        sps.profile_idc == 144) {  // old High444 profile

        sps.chroma_format_idc = gb.get_ue_golomb_31();
        if (sps.chroma_format_idc > 3) {
            DEBUG_SPS("chroma_format_idc is invalid");
            return false;
        }
            
        if (sps.chroma_format_idc == 3) {
            sps.residual_color_transform_flag = gb.getBits1();
            if (sps.residual_color_transform_flag != 0) {
                DEBUG_SPS("residual_color_transform_flag is invalid");
                return false;
            }
        }

        sps.bit_depth_luma   = gb.get_ue_golomb_31() + 8;
        sps.bit_depth_chroma = gb.get_ue_golomb_31() + 8;

        if(sps.bit_depth_chroma != sps.bit_depth_luma) {
            DEBUG_SPS("bit_depth_chroma bit_depth_luma not matched");
            return false;
        }

        if (sps.bit_depth_luma < 8 or sps.bit_depth_luma > 14) {
            DEBUG_SPS("bit_depth_luma is invalid");
            return false;
        }

        if (sps.bit_depth_chroma < 8 or sps.bit_depth_chroma > 14) {
            DEBUG_SPS("bit_depth_chroma is invalid");
            return false;
        }

        sps.transform_bypass = gb.getBits1();

        ret = decode_scaling_matrices(&gb, &sps, gb.getBits1(), &sps.scaling_matrix_present_mask,
                                    sps.scaling_matrix4, sps.scaling_matrix8);
        if (ret < 0) {
            DEBUG_SPS("decode_scaling_matrices error");
            return false;
        }

        sps.scaling_matrix_present |= ret;

    } else {
        sps.chroma_format_idc = 1;
        sps.bit_depth_luma    = 8;
        sps.bit_depth_chroma  = 8;
    }

    log2_max_frame_num_minus4 = gb.get_ue_golomb_31();
    if (log2_max_frame_num_minus4 < MIN_LOG2_MAX_FRAME_NUM - 4 or 
       log2_max_frame_num_minus4 > MAX_LOG2_MAX_FRAME_NUM - 4 ) {
        DEBUG_SPS("log2_max_frame_num_minus4 is invalid");
        return false;
    }

    sps.log2_max_frame_num = log2_max_frame_num_minus4 + 4;
    sps.poc_type = gb.get_ue_golomb_31();

    if (sps.poc_type == 0) { // FIXME #define
        unsigned t = gb.get_ue_golomb_31();
        if (t > 12) {
            DEBUG_SPS("t is invalid");
            return false;
        }
        sps.log2_max_poc_lsb = t + 4;        
    } else if (sps.poc_type == 1) { // FIXME #define
        sps.delta_pic_order_always_zero_flag = gb.getBits1();
        sps.offset_for_non_ref_pic           = gb.get_se_golomb_long();
        sps.offset_for_top_to_bottom_field   = gb.get_se_golomb_long();

        if (sps.offset_for_non_ref_pic         == INT32_MIN
            || sps.offset_for_top_to_bottom_field == INT32_MIN ) {
            DEBUG_SPS("offset_for_non_ref_pic is invalid");
            return false;
        }

        sps.poc_cycle_length = gb.get_ue_golomb();

        if ((unsigned)sps.poc_cycle_length >=
            (sizeof(sps.offset_for_ref_frame) / sizeof((sps.offset_for_ref_frame)[0]))) {
            DEBUG_SPS("poc_cycle_length is invalid");
            return false;
        }

        for (i = 0; i < sps.poc_cycle_length; i++) {
            sps.offset_for_ref_frame[i] = gb.get_se_golomb_long();
            if (sps.offset_for_ref_frame[i] == INT32_MIN) {
                DEBUG_SPS("offset_for_ref_frame is invalid");
                return false;
            }
        }
    } else if (sps.poc_type != 2) {
        DEBUG_SPS("poc_type is invalid");
        return false;
    }

    sps.ref_frame_count = gb.get_ue_golomb_31();
    
    if (sps.ref_frame_count > 16) {
        // 外面判断codec_tag
        DEBUG_SPS("ref_frame_count is invalid");
        return false;
    }
    sps.gaps_in_frame_num_allowed_flag = gb.getBits1();
    sps.mb_width                       = gb.get_ue_golomb() + 1;
    sps.mb_height                      = gb.get_ue_golomb() + 1;

    sps.frame_mbs_only_flag = gb.getBits1();

    if (sps.mb_height >= __INT_MAX__ / 2) {
        DEBUG_SPS("mb_height is invalid");
        return false;
    }
    sps.mb_height *= 2 - sps.frame_mbs_only_flag;

    if (!sps.frame_mbs_only_flag)
        sps.mb_aff = gb.getBits1();
    else
        sps.mb_aff = 0;

    sps.direct_8x8_inference_flag = gb.getBits1();

    sps.crop = gb.getBits1();
    if (sps.crop) {
        unsigned int crop_left   = gb.get_ue_golomb();
        unsigned int crop_right  = gb.get_ue_golomb();
        unsigned int crop_top    = gb.get_ue_golomb();
        unsigned int crop_bottom = gb.get_ue_golomb();
        //int width  = 16 * sps->mb_width;
        //int height = 16 * sps->mb_height;

        int vsub   = (sps.chroma_format_idc == 1) ? 1 : 0;
        int hsub   = (sps.chroma_format_idc == 1 ||
                        sps.chroma_format_idc == 2) ? 1 : 0;
        int step_x = 1 << hsub;
        int step_y = (2 - sps.frame_mbs_only_flag) << vsub;

        sps.crop_left   = crop_left   * step_x;
        sps.crop_right  = crop_right  * step_x;
        sps.crop_top    = crop_top    * step_y;
        sps.crop_bottom = crop_bottom * step_y;
        
    } else {
        sps.crop_left   =
        sps.crop_right  =
        sps.crop_top    =
        sps.crop_bottom =
        sps.crop        = 0;
    }

    sps.vui_parameters_present_flag = gb.getBits1();
    if (sps.vui_parameters_present_flag) {
        ret = decode_vui_parameters(&gb, &sps);
        if (ret < 0) {
            DEBUG_SPS("decode_vui_parameters is invalid");
            return false;
        }
    }

    if (gb.get_bits_left() < 0) {
        DEBUG_SPS("get_bits_left is invalid");
        return false;
    }

    /* if the maximum delay is not stored in the SPS, derive it based on the
     * level */
    if (!sps.bitstream_restriction_flag && (sps.ref_frame_count)) {
        sps.num_reorder_frames = 16 - 1;
        for (i = 0; i < HTO_INT(sizeof(level_max_dpb_mbs) / sizeof((level_max_dpb_mbs)[0])); i++) {
            if (level_max_dpb_mbs[i][0] == sps.level_idc) {
                sps.num_reorder_frames = ((level_max_dpb_mbs[i][1] / (sps.mb_width * sps.mb_height)) > 
                (sps.num_reorder_frames) ? (sps.num_reorder_frames) : (level_max_dpb_mbs[i][1] / (sps.mb_width * sps.mb_height)));
                break;
            }
        }
    }

    if (!sps.vui.sar.den)
        sps.vui.sar.den = 1;

    return true;
}