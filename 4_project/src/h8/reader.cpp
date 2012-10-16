#include "h8.h"

Reader::Reader(const char * s, size_t elementSize, bool autoClose) {
    this->elementSize = elementSize;
    auto size = fsize(s);
    panicIf(size % elementSize != 0, "file size error");
    n = size / elementSize;

    f = fopen(s, "rb");
    panicIf(!f, "open file");
    i = 0;
    this->autoClose = autoClose;
    this->closed = false;
}

Reader::Reader(FILE * f, size_t elementSize, bool autoClose) {
    this->elementSize = elementSize;
    assert(f);
    this->f = f;
    i = 0;
    this->autoClose = autoClose;
    this->closed = false;
}

bool Reader::read(void * buf) {
    auto ret = fread(buf, elementSize, 1, f);
    if (!ret && feof(f)) {
        if (autoClose) close();
        return false;
    }
    panicIf(ret != 1, "read file");
    i++;
    return true;
}

size_t Reader::batchRead(void * buf, size_t n) {
    assert(buf);
    auto ret = fread(buf, sizeof(elementSize), n, f);
    if (!ret && feof(f)) {
        if (autoClose) close();
        return 0;
    }
    panicIf(ret == 0, "read file");
    i += ret;
    return ret;
}

bool Reader::readLast(void * buf) {
    if (n == 0) return false;
    int ret = fseek(f, (n - 1) * sizeof(elementSize), SEEK_SET);
    panicIf(ret, "fseek on readLast");
    return read(buf);
}

void Reader::close() {
    if (!closed) { 
        fclose(f); 
        closed = true; 
    } 
}
