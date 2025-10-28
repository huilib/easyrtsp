

#include "h264packets.h"


namespace utils {


int is_h264_header(const uint8_t * header) {

    if (header[0] == 0x00 and header[1] == 0x00 
                and header[2] == 0x01) {
        return 3;
    } 
    if (header[0] == 0x00 and header[1] == 0x00 
            and header[2] == 0x00 and header[3] == 0x01) {
        return 4;
    }
    return 0;

}




void get_all_package(const uint8_t * data, int len, std::vector<std::vector<uint8_t>>& res) {

    for (int i = 0; i < len; ++i) {

        int h264_type = is_h264_header(data + i);

        if (h264_type == 0) {
            continue;
        }        

        const uint8_t* cpos = data + i + h264_type;
        std::vector<uint8_t> son_package;

        int pos_end = -1;
        for (int j = i + 2; j < len; ++j) {
            int h264_type_end = is_h264_header(data + j);
            if (h264_type_end == 3 or h264_type_end == 4) {
                pos_end = j;
                break;
            }
        }        

        if (pos_end == -1) {
            son_package.insert(son_package.end(), cpos, cpos + len - i - h264_type);
            //std::cout << "h264 type: " << h264_type << ",i: " << i << ", pos_end: " << pos_end << ", nalu length: " << son_package.size() << std::endl;
            res.emplace_back(son_package);
            return ;
        } else {
            //int nal_type = cpos[0] & 0x1f;
            son_package.insert(son_package.end(), cpos, cpos + pos_end - i - h264_type);
            //std::cout << "h264 type: " << h264_type << ",i: " << i << ", pos_end: " << pos_end << ", nalu length: " << son_package.size() << std::endl;
            res.emplace_back(son_package);            
            i = pos_end - 1;
        }                   
    
    }

}


unsigned removeH264or5EmulationBytes(uint8_t* to, unsigned toMaxSize, 
    const uint8_t * from, unsigned fromSize) {
    
    unsigned toSize = 0;
    unsigned i = 0;
    while (i < fromSize && toSize+1 < toMaxSize) {
        if (i+2 < fromSize && from[i] == 0 && from[i+1] == 0 && from[i+2] == 3) {
        to[toSize] = to[toSize+1] = 0;
        toSize += 2;
        i += 3;
        } else {
        to[toSize] = from[i];
        toSize += 1;
        i += 1;
        }
    }

    return toSize;

}


}

