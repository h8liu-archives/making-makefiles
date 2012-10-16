#include "navy.h"

using std::map;

static int tcmp(const void * p1, const void * p2) {
    const Access & a1 = *(const Access *)(p1);
    const Access & a2 = *(const Access *)(p2);

    if (a1.time < a2.time) return -1;
    if (a1.time > a2.time) return 1;
    return 0; 
}

void sortIntoTimeline(const char * in, const char * out) {
    ExtSorter sorter(sizeof(Access), tcmp);
    sorter.sort(in, out);
}

void hashSortIntoTimeline(const char * in, const char * out) {
    map<uint64_t, Writer *> files;

    Reader reader(in, sizeof(Access));
    Access a;
    while (reader.read(&a)) {
        uint64_t bin = a.time / (uint64_t(1e6) * 3600 * 24 * 2);
        auto it = files.find(bin);
        if (it == files.end()) {
            // create a new temp file
            files[bin] = new Writer(tmpfile(), sizeof(Access));
            it = files.find(bin);
        }
        it->second->write(&a);
    }
    
    Writer final(out, sizeof(Access));
    ExtSorter sorter(sizeof(Access), tcmp);

    for (auto ref : files) {
        // printf("bin: %ld\n", ref.first);
        FILE * fin = ref.second->cfile();
        rewind(fin); // rewind the file

        // external sort this file to ftmp
        FILE * ftmp = tmpfile();
        sorter.sort(fin, ftmp);
        fclose(fin); // fin is now removed

        // append to final
        rewind(ftmp);
        Reader reader(ftmp, sizeof(Access));
        while (reader.read(&a)) {
            final.write(&a);
        }
        // the reader will auto close ftmp
        
        delete ref.second; // the writer has no use now
    }

    final.close();
}

bool isTimeline(const char * in) {
    ExtSorter sorter(sizeof(Access), tcmp);
    return sorter.check(in);
}
