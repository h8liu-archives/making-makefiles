#include "h8.h"

IntQueue::IntQueue(size_t elementSize, size_t nbuf) {
    buf = (uint8_t *)(malloc(elementSize * nbuf));
    assert(buf);
    this->elementSize = elementSize;
    this->nbuf = nbuf * elementSize;
    this->head = 0;
    this->count = 0;
}

bool IntQueue::push(void * e) {
    if (full()) return false;
    void * t = &buf[(head + count) % nbuf];
    memcpy(t, e, elementSize);
    count += elementSize;
    return true;
}

bool IntQueue::pull(void * e) {
    if (empty()) return false;
    void * h = &buf[head];
    memcpy(e, h, elementSize);
    count -= elementSize;
    head += elementSize;
    assert(head <= nbuf);
    if (head == nbuf) head = 0;
    return true;
}

void IntQueue::clear() {
    head = 0;
    count = 0;
}

void IntQueue::dump(FILE * f) {
    assert(full());
    
    auto written = fwrite(&buf[head], 1, nbuf - head, f);
    assert(written == nbuf - head);
    if (head != 0) {
        written = fwrite(buf, 1, head, f);
        assert(written == head);
    }
    fflush(f);
}

void IntQueue::load(FILE * f) {
    auto read = fread(buf, 1, nbuf, f);
    assert(read == nbuf);
}

IntQueue::~IntQueue() {
    free(buf);
}

ExtQueue::ExtQueue(size_t elementSize, size_t nbuf)
    : elementSize(elementSize), 
    nbuf(nbuf), 
    headQueue(elementSize, nbuf),
    tailQueue(elementSize, nbuf) {

    transfer = (uint8_t *)(malloc(elementSize));
    assert(transfer);
}

void ExtQueue::push(void * e) {
    if (fileQueue.empty()) {
        if (headQueue.push(e)) 
            return;
    }

    if (tailQueue.push(e)) 
        return;
    
    FILE * f = tmpfile();
    tailQueue.dump(f);
    rewind(f);
    fileQueue.push_back(f);

    tailQueue.clear();
    bool ret = tailQueue.push(e);
    assert(ret);
    return;
}

bool ExtQueue::pull(void * e) {
    bool ret = headQueue.pull(e);
    if (ret) {
        if (fileQueue.empty()) {
            if (tailQueue.pull(transfer)) {
                // tailQueue is not empty
                // headQueue must be full
                headQueue.push(transfer);
                assert(headQueue.full());      
            }
        } else if (headQueue.empty()) { // file queue not empty
            FILE * f = fileQueue.front();
            headQueue.load(f);
            fclose(f);
            fileQueue.pop_front();
            assert(headQueue.full());
        }
    }

    return ret;
}

ExtQueue::~ExtQueue() {
    free(transfer);
}
