#include "h8.h"

Writer::Writer(const char *s, size_t elementSize) {
    this->elementSize = elementSize;
    f = fopen(s, "wb");
    panicIf(!f, "create file");
}

Writer::Writer(FILE * f, size_t elementSize) {
    this->elementSize = elementSize;
    assert(f);
    this->f = f;
}

void Writer::write(void * buf) {
    auto ret = fwrite(buf, elementSize, 1, f);
    panicIf(ret != 1, "write file");
}
