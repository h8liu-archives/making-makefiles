#include "h8.h"

static inline uint64_t & datref(std::vector<uint64_t> & dat, uint64_t i) {
    return dat[i / 64];
}

static inline uint64_t datbit(uint64_t i) {
    return 0x1ul << (i % 64);
}

Bitmap::Bitmap(uint64_t size) : 
    n(0) {
    reserve(size);
}

bool Bitmap::get(uint64_t i) {
    if (i >= n) return false;
    return (datref(dat, i) & datbit(i)) != 0;
}

void Bitmap::set(uint64_t i, bool b) {
    if (b) {
        reserve(i);
        datref(dat, i) |= datbit(i);
        return;
    } 

    if (i >= n) return;
    datref(dat, i) &= ~datbit(i);
}

void Bitmap::reserve(uint64_t i) {
    if (i < n) return;

    n = i;
    if (i % 64 != 0) i += 64 - (n % 64);  
    dat.resize(i / 64, 0);
}
