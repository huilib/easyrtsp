

#ifndef __H_BITREADER_H__
#define __H_BITREADER_H__

#include <huicpp.h>


using namespace HUICPP;


class BitReader {
public:
    const uint8_t *buffer, *buffer_end;
    int index;
    int size_in_bits;
    int size_in_bits_plus8;

    BitReader(const uint8_t *buf, int len);

    int get_bits_count() const { return index; }

    int get_bits_left() const { 
        return size_in_bits - get_bits_count();
    }

    unsigned int getBits1();

    unsigned int getBits(int n);

    void skip_bits(int n) {
        index += (n);
    }

    void skip_bits_long(int n){
        index += n;
    }


    unsigned get_ue_golomb_long();


    int get_ue_golomb_31();


    int get_ue_golomb();

    int get_se_golomb();

    int get_se_golomb_long();


    unsigned int show_bits(int n);


    unsigned int show_bits_long(int n);


    unsigned int get_bits_long(int n);
};


#endif // __H_BITREADER_H__


