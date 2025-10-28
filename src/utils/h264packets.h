
#ifndef __H_H264_PACKET_H__
#define __H_H264_PACKET_H__

#include <vector>
#include <cstdint>

namespace utils {

int is_h264_header(const uint8_t * header);

void get_all_package(const uint8_t * data, int len, std::vector<std::vector<uint8_t>>& res);

unsigned removeH264or5EmulationBytes(uint8_t* to, unsigned toMaxSize,
                                     const uint8_t * from, unsigned fromSize);

}


#endif //__H_H264_PACKET_H__