#include "h8.h"

static inline uint64_t index(uint32_t x, uint32_t y) { 
    return ((uint64_t)(x) << 32) + y; 
}
static inline uint32_t getx(uint64_t ind) { 
    return (uint32_t)(ind >> 32); 
}
static inline uint32_t gety(uint64_t ind) { 
    return (uint32_t)(ind & ((0x1l << 32) - 1)); 
}
    
Scatter::Scatter() { 
    reset(); 
}

void Scatter::reset(uint32_t x, uint32_t y) {
    xshift = 0; yshift = 0;
    xmax = x; ymax = y;
    points.clear();
}

void Scatter::xmerge() {
    std::map<uint64_t, uint64_t> merged;
    for (auto it = points.begin(); it != points.end(); it++) {
        uint64_t ind = it->first;
        uint64_t x = getx(ind);
        uint64_t y = gety(ind);
        uint64_t newind = index(x / 2, y);

        if (x & 0x1) {
            auto other = points.find(index(x - 1, y));
            if (other == points.end()) 
                merged[newind] = it->second;
            else
                merged[newind] = it->second + other->second;
        } else {
            auto other = points.find(index(x + 1, y));
            if (other == points.end()) 
                merged[newind] = it->second;
        }
    }
    points.swap(merged);
}

void Scatter::ymerge() {
    std::map<uint64_t, uint64_t> merged;
    for (auto it = points.begin(); it != points.end(); it++) {
        uint64_t ind = it->first;
        uint32_t x = getx(ind);
        uint32_t y = gety(ind);
        uint64_t newind = index(x, y / 2);

        if (y & 0x1) {
            auto other = points.find(index(x, y - 1));
            if (other == points.end()) 
                merged[newind] = it->second;
            else
                merged[newind] = it->second + other->second;
        } else {
            auto other = points.find(index(x, y + 1));
            if (other == points.end())
                merged[newind] = it->second;
        }
    }
    points.swap(merged);
}

void Scatter::count(uint64_t x, uint64_t y, uint64_t w) {
    x >>= xshift;
    y >>= yshift;
    while (x >= xmax) {
        xmerge();
        xshift++; x >>= 1;
    }

    while (y >= ymax) {
        ymerge();
        yshift++; y >>= 1;
    }

    uint64_t ind = index(x, y);
    auto it = points.find(ind);
    if (it == points.end())
        points[ind] = w;
    else
        it->second += w;
}

void Scatter::freport(FILE * out) {
    fprintf(out, "%ld\n", points.size());
    for (auto it = points.begin(); it != points.end(); it++) {
        uint64_t ind = it->first;
        uint64_t x = (uint64_t)(getx(ind));
        uint64_t y = (uint64_t)(gety(ind));
        x <<= xshift; y <<= yshift;
        fprintf(out, "%ld %ld %ld\n", x, y, it->second);
    }
}
