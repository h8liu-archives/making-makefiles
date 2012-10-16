#include "h8.h"

#include <queue>
using std::priority_queue;
using std::vector;

struct FilePeeker {
    FILE * file;
    size_t elementSize;
    void * head;
    int (*cmp)(const void *, const void *);

    FilePeeker(FILE * file, size_t elementSize, 
            int (*cmp)(const void *, const void *)) : 
        file(file), elementSize(elementSize), 
        head(malloc(elementSize)), cmp(cmp)
    { 
        assert(head);
        auto have = readNext();
        assert(have);
    }
    ~FilePeeker() { free(head); }

    void close() { fclose(file); }

    const void * peek() const { return head; }
    bool readNext() { 
        auto read = fread(head, elementSize, 1, file);
        if (read == 0) { assert(feof(file)); return false; }
        return true;
    }
};

struct FilePeekComparer : 
    public std::binary_function<FilePeeker *, FilePeeker *, bool> {
    // we need the smaller one pop first, so when a < b, return false;
    bool operator() (const FilePeeker * a, const FilePeeker * b) const {
        return a->cmp(a->peek(), b->peek()) > 0;
    }
};

void ExtSorter::sort(const char * pathin, const char * pathout) {
    FILE * fin = fopen(pathin, "rb");
    assert(fin);
    FILE * fout = fopen(pathout, "wb");
    assert(fout);
    this->sort(fin, fout);
    fclose(fin);
    fclose(fout);
}

uint64_t ExtSorter::intSort(FILE * fin, void * buf, uint64_t n, FILE * fout) {
    auto read = fread(buf, elementSize, n, fin);
    if (read == 0) {
        assert(feof(fin));
        return 0;
    }
    qsort(buf, read, elementSize, cmp);
    auto written = fwrite(buf, elementSize, read, fout);
    assert(written == read);
    fflush(fout);

    return read;
}

void ExtSorter::sort(FILE * fin, FILE * fout) {
    assert(fin); assert(fout);
    size_t n = memSize / elementSize;

    uint64_t inputSize = fsize(fin);
    assert(inputSize % elementSize == 0);
    uint64_t ninput = inputSize / elementSize;
    if (ninput <= n) {
        // printf("short cutting, only 1 IO round\n");
        void * buf = malloc(ninput * elementSize);
        auto sorted = intSort(fin, buf, ninput, fout);
        assert(sorted == ninput);
        free(buf);
        return;
    }

    priority_queue<FilePeeker *, vector<FilePeeker *>, FilePeekComparer> 
        files;
    
    void * buf = malloc(n * elementSize);
    assert(buf);
    // first round IO
    uint64_t nleft = ninput;
    while (nleft > 0) {
        FILE * ftmp = tmpfile();
        uint64_t sorted = intSort(fin, buf, n, ftmp);
        assert( (nleft <= n) ? (sorted == nleft) : (sorted == n) );
        
        rewind(ftmp);
        FilePeeker * peeker = new FilePeeker(ftmp, elementSize, cmp);
        files.push(peeker);

        nleft -= sorted;
    }
    free(buf);

    // second round IO
    while (!files.empty()) {
        FilePeeker * top = files.top();
        files.pop();
        auto written = fwrite(top->peek(), elementSize, 1, fout);
        assert(written == 1);
        if (top->readNext()) {
            files.push(top);
        } else {
            top->close();
            delete top;
        }
    }

    fflush(fout);
}

bool ExtSorter::check(const char * pathin) {
    Reader reader(pathin, elementSize);
    void * buf1 = malloc(elementSize);
    void * buf2 = malloc(elementSize);
    assert(buf1 && buf2);

    bool ret = true;
    if (reader.read(buf1)) {
        while (1) {
            if (!reader.read(buf2)) break;
            if (cmp(buf1, buf2) > 0) {
                ret = false; break;
            }

            void * hold = buf1;
            buf1 = buf2;
            buf2 = hold;
        }
    }

    free(buf1); free(buf2);
    return ret;
}

ExtSorter::ExtSorter(
        size_t elementSize,
        int (*cmp)(const void * a, const void * b),
        size_t memSize
        ) {
    this->elementSize = elementSize;
    this->memSize = memSize;
    this->cmp = cmp; 
}
